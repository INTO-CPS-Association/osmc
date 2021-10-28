package org.intocps.maestro.fmi2extension;

import org.intocps.maestro.ast.node.PStm;
import org.intocps.maestro.framework.fmi2.api.Fmi2Builder;
import org.intocps.maestro.framework.fmi2.api.mabl.MablApiBuilder;
import org.intocps.maestro.framework.fmi2.api.mabl.variables.DoubleVariableFmi2Api;

public class Fmi2ExtensionApi {

        final Fmi2Builder.RuntimeModule<PStm> module;

        final Fmi2Builder.RuntimeFunction getMaxStepSizeFunction;

        public Fmi2ExtensionApi(MablApiBuilder builder, Fmi2Builder.RuntimeModule<PStm> module) {
            this.module = module;

            getMaxStepSizeFunction = builder.getFunctionBuilder().setName("getMaxStepSize").addArgument("fmi2component", "FMI2Component")
                    .setReturnType(Fmi2Builder.RuntimeFunction.FunctionType.Type.Double).build();
            module.initialize(getMaxStepSizeFunction);
        }

        public DoubleVariableFmi2Api getMaxStepSize(Fmi2Builder.Fmi2ComponentVariable componentVariable) {
            Fmi2Builder.Variable v = module.call(getMaxStepSizeFunction, componentVariable);
            return (DoubleVariableFmi2Api) v;

        }
}
