package org.intocps.maestro.fmi2extension;

import com.spencerwi.either.Either;

import org.intocps.maestro.interpreter.api.IValueLifecycleHandler;
import org.intocps.maestro.interpreter.values.ExternalModuleValue;
import org.intocps.maestro.interpreter.values.FunctionValue;
import org.intocps.maestro.interpreter.values.RealValue;
import org.intocps.maestro.interpreter.values.Value;
import org.intocps.maestro.interpreter.values.fmi.FmuComponentValue;
import org.intocps.fmi.FmiInvalidNativeStateException;
import org.intocps.fmi.FmuResult;

import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


@IValueLifecycleHandler.ValueLifecycle(name = "FMI2Extension")
public class FmiExtensionRuntimeModule implements IValueLifecycleHandler {

    static final Logger logger = LoggerFactory.getLogger(FmiExtensionRuntimeModule.class);

    public static class FmiExtensionModule extends ExternalModuleValue<Object> {

        public FmiExtensionModule() {
            super(createMembers(), null);
        }

        private static Map<String, Value> createMembers() {
            Map<String, Value> members = new HashMap<>();
            members.put("getMaxStepSize", new FunctionValue.ExternalFunctionValue(fargs -> {

                List<Value> args = fargs.stream().map(Value::deref).collect(Collectors.toList());

                checkArgLength(args, 1);

                Value compVal = args.get(0);

                FmuComponentValue comp = (FmuComponentValue) compVal;
                double returnResult = Double.MAX_VALUE;
                try {
                    FmuResult<Double> result = comp.getModule().getMaxStepSize();
                    returnResult = result.result;
                } catch (FmiInvalidNativeStateException e) {
                    logger.error("Failed to retrieve maxStepSize from " + compVal.toString(), e);
                }
                return new RealValue(returnResult);
            }));

            return members;
        }
    }

    @Override
    public Either<Exception, Value> instantiate(List<Value> list) {

        List<Value> args = list.stream().map(Value::deref).collect(Collectors.toList());

        if(!args.isEmpty()){
            return Either.left(new Exception("getMaxStepSize must be instantiated with 0 arguments."));
        }

        return Either.right(new FmiExtensionModule());
    }

    @Override
    public void destroy(Value value) {

    }

    @Override
    public InputStream getMablModule() {
        return this.getClass().getResourceAsStream("org/intocps/maestro/fmi2extension/FMI2Extension.mabl");
    }
}
