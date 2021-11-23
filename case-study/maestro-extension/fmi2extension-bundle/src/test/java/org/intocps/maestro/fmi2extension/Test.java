package org.intocps.maestro.fmi2extension;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.intocps.maestro.Mabl;
import org.intocps.maestro.ast.analysis.AnalysisException;
import org.intocps.maestro.ast.display.PrettyPrinter;
import org.intocps.maestro.ast.node.AIdentifierExp;
import org.intocps.maestro.ast.node.ASimulationSpecificationCompilationUnit;
import org.intocps.maestro.ast.node.PStm;
import org.intocps.maestro.core.Framework;
import org.intocps.maestro.core.messages.ErrorReporter;
import org.intocps.maestro.core.messages.IErrorReporter;
import org.intocps.maestro.framework.fmi2.api.Fmi2Builder;
import org.intocps.maestro.framework.fmi2.api.mabl.*;
import org.intocps.maestro.framework.fmi2.api.mabl.scoping.DynamicActiveBuilderScope;
import org.intocps.maestro.framework.fmi2.api.mabl.scoping.IfMaBlScope;
import org.intocps.maestro.framework.fmi2.api.mabl.scoping.WhileMaBLScope;
import org.intocps.maestro.framework.fmi2.api.mabl.values.IntExpressionValue;
import org.intocps.maestro.framework.fmi2.api.mabl.variables.*;
import org.intocps.maestro.interpreter.DefaultExternalValueFactory;
import org.intocps.maestro.interpreter.MableInterpreter;
import org.junit.Assert;
import org.reflections.Reflections;
import org.reflections.scanners.ResourcesScanner;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;

public class Test {
    double MAXIMUM_STEP_SIZE = 6.0;
    double MINIMUM_STEP_SIZE = 0.1;
    public static String outputs_csv = "outputs.csv";
    @org.junit.Test
    public void initialTest() throws Exception {
        MablApiBuilder builder = new MablApiBuilder();
        DynamicActiveBuilderScope scope = builder.getDynamicScope();
        DoubleVariableFmi2Api startTime = scope.store("startTime", 0.0);
        DoubleVariableFmi2Api endTime = scope.store("endTime", 10.0);
        DoubleVariableFmi2Api step_size_minimum = scope.store("step_size_minimum", MINIMUM_STEP_SIZE);
        DoubleVariableFmi2Api step_size_maximum = scope.store("step_size_maximum", MAXIMUM_STEP_SIZE);
        DoubleVariableFmi2Api step_size_default = scope.store("step_size_default", 0.1);
        BooleanVariableFmi2Api mitigate = scope.store("mitigate", false);
        DoubleVariableFmi2Api currentCommunicationPoint = scope.store("currentCommunicationPoint", 0.0);
        DoubleVariableFmi2Api current_step_size = scope.store("current_step_size", 0.0);
        current_step_size.setValue(step_size_default);

        File osmcFMUFile = new File(this.getClass().getResource("/fmi2extension/osmc.fmu").getFile());
        File rabbitMQFMUFile = new File(this.getClass().getResource("/fmi2extension/rabbitmq_cs_1.fmu").getFile());

        // Load the fmi2Exntesion runtime module
        RuntimeModuleVariable fmi2Extension = builder.loadRuntimeModule("FMI2Extension");
        Fmi2ExtensionApi fmi2ExtensionApi = new Fmi2ExtensionApi(builder, fmi2Extension);

        FmuVariableFmi2Api osmcFMU = scope.createFMU("osmc", "FMI2", URI.create(osmcFMUFile.getAbsolutePath()).toString());
        ComponentVariableFmi2Api osmcInstance = osmcFMU.instantiate("osmcInstance");
        osmcInstance.setupExperiment(startTime, endTime, null);
        osmcInstance.enterInitializationMode();
        osmcInstance.getAndShare();
        osmcInstance.exitInitializationMode();
        // OSMCInstance ready



        FmuVariableFmi2Api rbmqFMU = scope.createFMU("rbmqfmu", "FMI2", URI.create(rabbitMQFMUFile.getAbsolutePath()).toString());
        ComponentVariableFmi2Api rbmqInstance = rbmqFMU.instantiate("rbmqInstance");

        // Link the ports
        PortFmi2Api osmcOutOfSyncPort = osmcInstance.getPort("outofsync");
        PortFmi2Api rbmqFmuOutOfSyncPort = rbmqInstance.getPort("outofsync");
        osmcOutOfSyncPort.linkTo(rbmqFmuOutOfSyncPort);

        rbmqInstance.setupExperiment(startTime, endTime, null);
        PortFmi2Api maxage = rbmqInstance.getPort("config.maxage");
        rbmqInstance.set(maxage, new IntExpressionValue(200));
        rbmqInstance.enterInitializationMode();
        rbmqInstance.setLinked();
        rbmqInstance.exitInitializationMode();
        // RBMQInstance ready

        DataWriter dataWriter = builder.getDataWriter();
        DataWriter.DataWriterInstance dataWriterInstance = dataWriter.createDataWriterInstance();

        PortFmi2Api rbmqFmuSeqnoPort = rbmqInstance.getPort("seqno");
        rbmqInstance.getAndShare(rbmqFmuSeqnoPort);
        dataWriterInstance.initialize(osmcOutOfSyncPort, rbmqFmuSeqnoPort);

        dataWriterInstance.log(currentCommunicationPoint);

        DoubleVariableFmi2Api osmcMaxStepSizeVariable = scope.store("osmcMaxStepSize", 0.0);
        osmcMaxStepSizeVariable.setValue(step_size_default);
        DoubleVariableFmi2Api rbmqMaxStepSizeVariable = scope.store("rbmqMaxStepSize", 0.0);
        rbmqMaxStepSizeVariable.setValue(step_size_default);


        WhileMaBLScope simulationLoop = scope.enterWhile(currentCommunicationPoint.toMath().addition(current_step_size).lessEqualTo(endTime));
        {
            ConsolePrinter consolePrinter = builder.getConsolePrinter();
            consolePrinter.print("time %f", currentCommunicationPoint);
            // Step the FMUs
            builder.getLogger().log(LoggerFmi2Api.Level.ERROR, "current step size %f", current_step_size);

            Map.Entry<Fmi2Builder.BoolVariable<PStm>, Fmi2Builder.DoubleVariable<PStm>> step = osmcInstance.step(currentCommunicationPoint, current_step_size);
            Map.Entry<Fmi2Builder.BoolVariable<PStm>, Fmi2Builder.DoubleVariable<PStm>> step2 = rbmqInstance.step(currentCommunicationPoint, current_step_size);

            currentCommunicationPoint.setValue(currentCommunicationPoint.toMath().addition(current_step_size));
            // Get state of all FMUs
            rbmqInstance.getAndShare();
            osmcInstance.getAndShare();

            // set mitigate = out of sync
            mitigate.setValue(osmcOutOfSyncPort.getSharedAsVariable());

            dataWriterInstance.log(currentCommunicationPoint);

            // Calculate the step size
            Runnable getMaxStep = () -> {
                var tempOsmcMaxStepSize = fmi2ExtensionApi.getMaxStepSize(osmcInstance);
                osmcMaxStepSizeVariable.setValue(tempOsmcMaxStepSize);
                var temprbmqMaxStepSizeVariable = fmi2ExtensionApi.getMaxStepSize(rbmqInstance);
                rbmqMaxStepSizeVariable.setValue(temprbmqMaxStepSizeVariable);
                IfMaBlScope ifrbmqLessThanOsmc = scope.enterIf(rbmqMaxStepSizeVariable.toMath().lessThan(osmcMaxStepSizeVariable));
                ifrbmqLessThanOsmc.enterThen();
                builder.getLogger().log(LoggerFmi2Api.Level.ERROR, "rabbitmq maxstep %f", rbmqMaxStepSizeVariable);
                current_step_size.setValue(rbmqMaxStepSizeVariable);
                scope.leave();
                ifrbmqLessThanOsmc.enterElse();
                builder.getLogger().log(LoggerFmi2Api.Level.ERROR, "osmc maxstep %f", osmcMaxStepSizeVariable);
                current_step_size.setValue(osmcMaxStepSizeVariable);
                scope.leave();

                IfMaBlScope ifMaBlScope = scope.enterIf(current_step_size.toMath().greaterThan(step_size_maximum));
                ifMaBlScope.enterThen();

                builder.getLogger().log(LoggerFmi2Api.Level.ERROR, "setting maximum %f", step_size_maximum);
                current_step_size.setValue(step_size_maximum);
                scope.leave();
                ifMaBlScope.enterElse();
                scope.enterIf(current_step_size.toMath().lessThan(step_size_minimum)).enterThen();
                current_step_size.setValue(step_size_minimum);
                scope.leave();



            };
            Runnable useDefault = () -> {
                current_step_size.setValue(step_size_default);
            };

            IfMaBlScope ifMitigate = scope.enterIf(mitigate.toPredicate());
            ifMitigate.enterThen();
            {
                getMaxStep.run();
            }
            ifMitigate.enterElse();
            {
                useDefault.run();
            }
            scope.leave();

        }
        scope.leave();

        osmcInstance.terminate();
        rbmqInstance.terminate();

        dataWriterInstance.close();
        ASimulationSpecificationCompilationUnit spec = builder.build();
        System.out.println(PrettyPrinter.print(spec));

        check(spec, true);


        //scope.enterIf(outOfSyncMode.toPredicate());
    }

    private void check(ASimulationSpecificationCompilationUnit spec, boolean execute) throws Exception {

        File workingDirectory = new File("target","testResult");
        workingDirectory.mkdirs();
        File specFile = new File(workingDirectory, "m.mabl");
        FileUtils.write(specFile, PrettyPrinter.print(spec), StandardCharsets.UTF_8);
        Mabl mabl = new Mabl(workingDirectory, workingDirectory);
        IErrorReporter reporter = new ErrorReporter();
        mabl.setReporter(reporter);
        mabl.setVerbose(true);
        String fmi2ExtensionMablFileName = "FMI2Extension.mabl";
        File fmi2extensionmabl = new File(workingDirectory, fmi2ExtensionMablFileName);

        if(!fmi2extensionmabl.exists())
        {
            InputStream stream = Fmi2ExtensionApi.class.getClassLoader().getResourceAsStream("org/intocps/maestro/fmi2extension/FMI2Extension.mabl");
            if(stream != null)
                FileUtils.copyInputStreamToFile(stream, fmi2extensionmabl);
            else
                throw new Exception("Failed to retrieve FMI2Extension.mabl");
        }

        mabl.parse(Arrays.asList(specFile, fmi2extensionmabl));

        mabl.typeCheck();
        mabl.verify(Framework.FMI2);
        if (reporter.getErrorCount() > 0) {
            reporter.printErrors(new PrintWriter(System.err, true));
            throw new RuntimeException("error");
        }

        Map<String, List<Map<String, Object>>> runtime_config = new HashMap<>();
        runtime_config.put("DataWriter", Arrays.asList(new HashMap<>() {
            {
                put("type", "CSV");
                put("filename", outputs_csv);
            }
        }));
        String configString = new ObjectMapper().writeValueAsString(runtime_config);

        mabl.dump(workingDirectory);
        if(execute)
        {
        new MableInterpreter(
                new DefaultExternalValueFactory(workingDirectory, IOUtils.toInputStream(configString, StandardCharsets.UTF_8)))
                .execute(mabl.getMainSimulationUnit());
        }
    }
}
