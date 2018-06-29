from openmdao.api import Component
import numpy as np
import wind_obos as wo

        
class WindOBOS(Component):
    """
    OpenMDAO Component class for mooring system attached to sub-structure of floating offshore wind turbines.
    Should be tightly coupled with Spar class for full system representation.
    """

    def __init__(self):
        super(WindOBOS,self).__init__()

        # Internal python-wobos object
        self.mywobos = wo.wobos()

        # Go through all variables from wobos text file of defaults and add as either inputs or outputs
        for k in xrange(len(wo.wobos_vars)):
            passBy = not (type(wo.wobos_vars[k][-2]) == type(0.0))
            if wo.wobos_vars[k][1] in ['moorLines']:
                passBy = False
            if wo.wobos_vars[k][0].upper() == 'INPUT':
                self.add_param(wo.wobos_vars[k][1], desc=wo.wobos_vars[k][2], units=wo.wobos_vars[k][3], val=wo.wobos_vars[k][-2], pass_by_obj=passBy)
            elif wo.wobos_vars[k][0].upper() == 'OUTPUT':
                self.add_output(wo.wobos_vars[k][1], desc=wo.wobos_vars[k][2], units=wo.wobos_vars[k][3], val=wo.wobos_vars[k][-2], pass_by_obj=passBy)

        # Derivatives
        self.deriv_options['type'] = 'fd'
        self.deriv_options['form'] = 'central'
        self.deriv_options['step_calc'] = 'relative'
        self.deriv_options['step_size'] = 1e-5

    def solve_nonlinear(self, params, unknowns, resids):
        '''Sets mooring line properties then writes MAP input file and executes MAP.
        
        INPUTS:
        ----------
        params   : dictionary of input parameters
        unknowns : dictionary of output parameters
        
        OUTPUTS  : none (all unknown dictionary values set)
        '''

        # Store local variables in wobos structure
        for k in params.keys():
            self.mywobos.variable_access(k, params[k])
            
        # Run model
        self.mywobos.run()

        # Extract outputs
        for k in unknowns.keys():
            unknowns[k] = self.mywobos.variable_access(k)
