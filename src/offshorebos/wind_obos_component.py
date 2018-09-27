from openmdao.api import Component, Group, IndepVarComp
import numpy as np
from .wind_obos import wobos, wobos_vars

        
class WindOBOSComp(Component):
    """
    OpenMDAO Component class for mooring system attached to sub-structure of floating offshore wind turbines.
    Should be tightly coupled with Spar class for full system representation.
    """

    def __init__(self):
        super(WindOBOSComp,self).__init__()

        # Internal python-wobos object
        self.mywobos = wobos()

        # Go through all variables from wobos text file of defaults and add as either inputs or outputs
        for k in range(len(wobos_vars)):
            passBy = not (type(wobos_vars[k][-2]) == type(0.0))
            if wobos_vars[k][1] in ['moorLines']:
                passBy = False
            if wobos_vars[k][0].upper() == 'INPUT':
                self.add_param(wobos_vars[k][1], desc=wobos_vars[k][2], units=wobos_vars[k][3], val=wobos_vars[k][-2], pass_by_obj=passBy)
            elif wobos_vars[k][0].upper() == 'OUTPUT':
                self.add_output(wobos_vars[k][1], desc=wobos_vars[k][2], units=wobos_vars[k][3], val=wobos_vars[k][-2], pass_by_obj=passBy)

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


class WindOBOS(Group):

    def __init__(self):
        super(WindOBOS,self).__init__()

        self.add('wobos', WindOBOSComp(), promotes=['*'])

        # Offshore BOS

        # Turbine / Plant parameters, ,
        self.add('nacelleL',                     IndepVarComp('nacelleL', 0.0), promotes=['*'])
        self.add('nacelleW',                     IndepVarComp('nacelleW', 0.0), promotes=['*'])
        self.add('distShore',                    IndepVarComp('distShore', 0.0), promotes=['*']) #90.0
        self.add('distPort',                     IndepVarComp('distPort', 0.0), promotes=['*']) #90.0
        self.add('distPtoA',                     IndepVarComp('distPtoA', 0.0), promotes=['*']) #90.0
        self.add('distAtoS',                     IndepVarComp('distAtoS', 0.0), promotes=['*']) #90.0
        self.add('substructure',                 IndepVarComp('substructure', 'SEMISUBMERSIBLE', pass_by_obj=True), promotes=['*'])
        self.add('anchor',                       IndepVarComp('anchor', 'DRAGEMBEDMENT', pass_by_obj=True), promotes=['*'])
        self.add('turbInstallMethod',            IndepVarComp('turbInstallMethod', 'INDIVIDUAL', pass_by_obj=True), promotes=['*'])
        self.add('towerInstallMethod',           IndepVarComp('towerInstallMethod', 'ONEPIECE', pass_by_obj=True), promotes=['*'])
        self.add('installStrategy',              IndepVarComp('installStrategy', 'PRIMARYVESSEL', pass_by_obj=True), promotes=['*'])
        self.add('cableOptimizer',               IndepVarComp('cableOptimizer', False, pass_by_obj=True), promotes=['*'])
        self.add('buryDepth',                    IndepVarComp('buryDepth', 0.0), promotes=['*']) #2.0
        self.add('arrayY',                       IndepVarComp('arrayY', 0.0), promotes=['*']) #9.0
        self.add('arrayX',                       IndepVarComp('arrayX', 0.0), promotes=['*']) #9.0
        self.add('substructCont',                IndepVarComp('substructCont', 0.0), promotes=['*']) #0.30
        self.add('turbCont',                     IndepVarComp('turbCont', 0.0), promotes=['*']) #0.30
        self.add('elecCont',                     IndepVarComp('elecCont', 0.0), promotes=['*']) #0.30
        self.add('interConVolt',                 IndepVarComp('interConVolt', 0.0), promotes=['*']) #345.0
        self.add('distInterCon',                 IndepVarComp('distInterCon', 0.0), promotes=['*']) #3.0
        self.add('scrapVal',                     IndepVarComp('scrapVal', 0.0), promotes=['*']) #0.0
        #General', , 
        self.add('inspectClear',                 IndepVarComp('inspectClear', 0.0), promotes=['*']) #2.0
        self.add('plantComm',                    IndepVarComp('plantComm', 0.0), promotes=['*']) #0.01
        self.add('procurement_contingency',      IndepVarComp('procurement_contingency', 0.0), promotes=['*']) #0.05
        self.add('install_contingency',          IndepVarComp('install_contingency', 0.0), promotes=['*']) #0.30
        self.add('construction_insurance',       IndepVarComp('construction_insurance', 0.0), promotes=['*']) #0.01
        self.add('capital_cost_year_0',          IndepVarComp('capital_cost_year_0', 0.0), promotes=['*']) #0.20
        self.add('capital_cost_year_1',          IndepVarComp('capital_cost_year_1', 0.0), promotes=['*']) #0.60
        self.add('capital_cost_year_2',          IndepVarComp('capital_cost_year_2', 0.0), promotes=['*']) #0.10
        self.add('capital_cost_year_3',          IndepVarComp('capital_cost_year_3', 0.0), promotes=['*']) #0.10
        self.add('capital_cost_year_4',          IndepVarComp('capital_cost_year_4', 0.0), promotes=['*']) #0.0
        self.add('capital_cost_year_5',          IndepVarComp('capital_cost_year_5', 0.0), promotes=['*']) #0.0
        self.add('tax_rate',                     IndepVarComp('tax_rate', 0.0), promotes=['*']) #0.40
        self.add('interest_during_construction', IndepVarComp('interest_during_construction', 0.0), promotes=['*']) #0.08
        #Substructure & Foundation', , 
        #self.add('ballast_cost_rate',            IndepVarComp('ballast_cost_rate', 0.0), promotes=['*'])
        #self.add('tapered_col_cost_rate',        IndepVarComp('tapered_col_cost_rate', 0.0), promotes=['*'])
        #self.add('outfitting_cost_rate',         IndepVarComp('outfitting_cost_rate', 0.0), promotes=['*'])
        #self.add('mpileD',                       IndepVarComp('mpileD', 0.0), promotes=['*']) #0.0
        #self.add('mpileL',                       IndepVarComp('mpileL', 0.0), promotes=['*']) #0.0
        #self.add('mpEmbedL',                     IndepVarComp('mpEmbedL', 0.0), promotes=['*']) #30.0
        self.add('mpileCR',                      IndepVarComp('mpileCR', 0.0), promotes=['*']) #2250.0
        self.add('mtransCR',                     IndepVarComp('mtransCR', 0.0), promotes=['*']) #3230.0
        self.add('jlatticeCR',                   IndepVarComp('jlatticeCR', 0.0), promotes=['*']) #4680.0
        self.add('jtransCR',                     IndepVarComp('jtransCR', 0.0), promotes=['*']) #4500.0
        self.add('jpileCR',                      IndepVarComp('jpileCR', 0.0), promotes=['*']) #2250.0
        self.add('jlatticeA',                    IndepVarComp('jlatticeA', 0.0), promotes=['*']) #26.0
        self.add('jpileL',                       IndepVarComp('jpileL', 0.0), promotes=['*']) #47.50
        self.add('jpileD',                       IndepVarComp('jpileD', 0.0), promotes=['*']) #1.60
        self.add('ssHeaveCR',                    IndepVarComp('ssHeaveCR', 0.0), promotes=['*']) #6250.0
        self.add('scourMat',                     IndepVarComp('scourMat', 0.0), promotes=['*']) #250000.0
        self.add('number_install_seasons',       IndepVarComp('number_install_seasons', 0, pass_by_obj=True), promotes=['*']) #1
        # Mooring
        self.add('deaFixLeng',                   IndepVarComp('deaFixLeng', 0.0), promotes=['*']) #250000.0
        #self.add('moorCR',                       IndepVarComp('moorCR', 0.0), promotes=['*']) #250000.0
        #self.add('moorCost',                     IndepVarComp('moorCost', 0.0), promotes=['*']) #250000.0
        #self.add('moorDia',                      IndepVarComp('moorDia', 0.0), promotes=['*']) #250000.0
        #self.add('moorLines',                    IndepVarComp('moorLines', 0), promotes=['*']) #250000.0
        #self.add('ssTrussCR',                    IndepVarComp('ssTrussCR', 0.0), promotes=['*']) #250000.0
        #Electrical Infrastructure', , 
        self.add('pwrFac',                       IndepVarComp('pwrFac', 0.0), promotes=['*']) #0.95
        self.add('buryFac',                      IndepVarComp('buryFac', 0.0), promotes=['*']) #0.10
        self.add('catLengFac',                   IndepVarComp('catLengFac', 0.0), promotes=['*']) #0.04
        self.add('exCabFac',                     IndepVarComp('exCabFac', 0.0), promotes=['*']) #0.10
        self.add('subsTopFab',                   IndepVarComp('subsTopFab', 0.0), promotes=['*']) #14500.0
        self.add('subsTopDes',                   IndepVarComp('subsTopDes', 0.0), promotes=['*']) #4500000.0
        self.add('topAssemblyFac',               IndepVarComp('topAssemblyFac', 0.0), promotes=['*']) #0.075
        self.add('subsJackCR',                   IndepVarComp('subsJackCR', 0.0), promotes=['*']) #6250.0
        self.add('subsPileCR',                   IndepVarComp('subsPileCR', 0.0), promotes=['*']) #2250.0
        self.add('dynCabFac',                    IndepVarComp('dynCabFac', 0.0), promotes=['*']) #2.0
        self.add('shuntCR',                      IndepVarComp('shuntCR', 0.0), promotes=['*']) #35000.0
        self.add('highVoltSG',                   IndepVarComp('highVoltSG', 0.0), promotes=['*']) #950000.0
        self.add('medVoltSG',                    IndepVarComp('medVoltSG', 0.0), promotes=['*']) #500000.0
        self.add('backUpGen',                    IndepVarComp('backUpGen', 0.0), promotes=['*']) #1000000.0
        self.add('workSpace',                    IndepVarComp('workSpace', 0.0), promotes=['*']) #2000000.0
        self.add('otherAncillary',               IndepVarComp('otherAncillary', 0.0), promotes=['*']) #3000000.0
        self.add('mptCR',                        IndepVarComp('mptCR', 0.0), promotes=['*']) #12500.0
        self.add('arrVoltage',                   IndepVarComp('arrVoltage', 0.0), promotes=['*']) #33.0
        self.add('cab1CR',                       IndepVarComp('cab1CR', 0.0), promotes=['*']) #185.889
        self.add('cab2CR',                       IndepVarComp('cab2CR', 0.0), promotes=['*']) #202.788
        self.add('cab1CurrRating',               IndepVarComp('cab1CurrRating', 0.0), promotes=['*']) #300.0
        self.add('cab2CurrRating',               IndepVarComp('cab2CurrRating', 0.0), promotes=['*']) #340.0
        self.add('arrCab1Mass',                  IndepVarComp('arrCab1Mass', 0.0), promotes=['*']) #20.384
        self.add('arrCab2Mass',                  IndepVarComp('arrCab2Mass', 0.0), promotes=['*']) #21.854
        self.add('cab1TurbInterCR',              IndepVarComp('cab1TurbInterCR', 0.0), promotes=['*']) #8410.0
        self.add('cab2TurbInterCR',              IndepVarComp('cab2TurbInterCR', 0.0), promotes=['*']) #8615.0
        self.add('cab2SubsInterCR',              IndepVarComp('cab2SubsInterCR', 0.0), promotes=['*']) #19815.0
        self.add('expVoltage',                   IndepVarComp('expVoltage', 0.0), promotes=['*']) #220.0
        self.add('expCurrRating',                IndepVarComp('expCurrRating', 0.0), promotes=['*']) #530.0
        self.add('expCabMass',                   IndepVarComp('expCabMass', 0.0), promotes=['*']) #71.90
        self.add('expCabCR',                     IndepVarComp('expCabCR', 0.0), promotes=['*']) #495.411
        self.add('expSubsInterCR',               IndepVarComp('expSubsInterCR', 0.0), promotes=['*']) #57500.0
        # Vector inputs
        #self.add('arrayCables',                  IndepVarComp('arrayCables', [33, 66], pass_by_obj=True), promotes=['*'])
        #self.add('exportCables',                 IndepVarComp('exportCables', [132, 220], pass_by_obj=True), promotes=['*'])
        #Assembly & Installation',
        self.add('moorTimeFac',                  IndepVarComp('moorTimeFac', 0.0), promotes=['*']) #0.005
        self.add('moorLoadout',                  IndepVarComp('moorLoadout', 0.0), promotes=['*']) #5.0
        self.add('moorSurvey',                   IndepVarComp('moorSurvey', 0.0), promotes=['*']) #4.0
        self.add('prepAA',                       IndepVarComp('prepAA', 0.0), promotes=['*']) #168.0
        self.add('prepSpar',                     IndepVarComp('prepSpar', 0.0), promotes=['*']) #18.0
        self.add('upendSpar',                    IndepVarComp('upendSpar', 0.0), promotes=['*']) #36.0
        self.add('prepSemi',                     IndepVarComp('prepSemi', 0.0), promotes=['*']) #12.0
        self.add('turbFasten',                   IndepVarComp('turbFasten', 0.0), promotes=['*']) #8.0
        self.add('boltTower',                    IndepVarComp('boltTower', 0.0), promotes=['*']) #7.0
        self.add('boltNacelle1',                 IndepVarComp('boltNacelle1', 0.0), promotes=['*']) #7.0
        self.add('boltNacelle2',                 IndepVarComp('boltNacelle2', 0.0), promotes=['*']) #7.0
        self.add('boltNacelle3',                 IndepVarComp('boltNacelle3', 0.0), promotes=['*']) #7.0
        self.add('boltBlade1',                   IndepVarComp('boltBlade1', 0.0), promotes=['*']) #3.50
        self.add('boltBlade2',                   IndepVarComp('boltBlade2', 0.0), promotes=['*']) #3.50
        self.add('boltRotor',                    IndepVarComp('boltRotor', 0.0), promotes=['*']) #7.0
        self.add('vesselPosTurb',                IndepVarComp('vesselPosTurb', 0.0), promotes=['*']) #2.0
        self.add('vesselPosJack',                IndepVarComp('vesselPosJack', 0.0), promotes=['*']) #8.0
        self.add('vesselPosMono',                IndepVarComp('vesselPosMono', 0.0), promotes=['*']) #3.0
        self.add('subsVessPos',                  IndepVarComp('subsVessPos', 0.0), promotes=['*']) #6.0
        self.add('monoFasten',                   IndepVarComp('monoFasten', 0.0), promotes=['*']) #12.0
        self.add('jackFasten',                   IndepVarComp('jackFasten', 0.0), promotes=['*']) #20.0
        self.add('prepGripperMono',              IndepVarComp('prepGripperMono', 0.0), promotes=['*']) #1.50
        self.add('prepGripperJack',              IndepVarComp('prepGripperJack', 0.0), promotes=['*']) #8.0
        self.add('placePiles',                   IndepVarComp('placePiles', 0.0), promotes=['*']) #12.0
        self.add('prepHamMono',                  IndepVarComp('prepHamMono', 0.0), promotes=['*']) #2.0
        self.add('prepHamJack',                  IndepVarComp('prepHamJack', 0.0), promotes=['*']) #2.0
        self.add('removeHamMono',                IndepVarComp('removeHamMono', 0.0), promotes=['*']) #2.0
        self.add('removeHamJack',                IndepVarComp('removeHamJack', 0.0), promotes=['*']) #4.0
        self.add('placeTemplate',                IndepVarComp('placeTemplate', 0.0), promotes=['*']) #4.0
        self.add('placeJack',                    IndepVarComp('placeJack', 0.0), promotes=['*']) #12.0
        self.add('levJack',                      IndepVarComp('levJack', 0.0), promotes=['*']) #24.0
        self.add('hamRate',                      IndepVarComp('hamRate', 0.0), promotes=['*']) #20.0
        self.add('placeMP',                      IndepVarComp('placeMP', 0.0), promotes=['*']) #3.0
        self.add('instScour',                    IndepVarComp('instScour', 0.0), promotes=['*']) #6.0
        self.add('placeTP',                      IndepVarComp('placeTP', 0.0), promotes=['*']) #3.0
        self.add('groutTP',                      IndepVarComp('groutTP', 0.0), promotes=['*']) #8.0
        self.add('tpCover',                      IndepVarComp('tpCover', 0.0), promotes=['*']) #1.50
        self.add('prepTow',                      IndepVarComp('prepTow', 0.0), promotes=['*']) #12.0
        self.add('spMoorCon',                    IndepVarComp('spMoorCon', 0.0), promotes=['*']) #20.0
        self.add('ssMoorCon',                    IndepVarComp('ssMoorCon', 0.0), promotes=['*']) #22.0
        self.add('spMoorCheck',                  IndepVarComp('spMoorCheck', 0.0), promotes=['*']) #16.0
        self.add('ssMoorCheck',                  IndepVarComp('ssMoorCheck', 0.0), promotes=['*']) #12.0
        self.add('ssBall',                       IndepVarComp('ssBall', 0.0), promotes=['*']) #6.0
        self.add('surfLayRate',                  IndepVarComp('surfLayRate', 0.0), promotes=['*']) #375.0
        self.add('cabPullIn',                    IndepVarComp('cabPullIn', 0.0), promotes=['*']) #5.50
        self.add('cabTerm',                      IndepVarComp('cabTerm', 0.0), promotes=['*']) #5.50
        self.add('cabLoadout',                   IndepVarComp('cabLoadout', 0.0), promotes=['*']) #14.0
        self.add('buryRate',                     IndepVarComp('buryRate', 0.0), promotes=['*']) #125.0
        self.add('subsPullIn',                   IndepVarComp('subsPullIn', 0.0), promotes=['*']) #48.0
        self.add('shorePullIn',                  IndepVarComp('shorePullIn', 0.0), promotes=['*']) #96.0
        self.add('landConstruct',                IndepVarComp('landConstruct', 0.0), promotes=['*']) #7.0
        self.add('expCabLoad',                   IndepVarComp('expCabLoad', 0.0), promotes=['*']) #24.0
        self.add('subsLoad',                     IndepVarComp('subsLoad', 0.0), promotes=['*']) #60.0
        self.add('placeTop',                     IndepVarComp('placeTop', 0.0), promotes=['*']) #24.0
        self.add('pileSpreadDR',                 IndepVarComp('pileSpreadDR', 0.0), promotes=['*']) #2500.0
        self.add('pileSpreadMob',                IndepVarComp('pileSpreadMob', 0.0), promotes=['*']) #750000.0
        self.add('groutSpreadDR',                IndepVarComp('groutSpreadDR', 0.0), promotes=['*']) #3000.0
        self.add('groutSpreadMob',               IndepVarComp('groutSpreadMob', 0.0), promotes=['*']) #1000000.0
        self.add('seaSpreadDR',                  IndepVarComp('seaSpreadDR', 0.0), promotes=['*']) #165000.0
        self.add('seaSpreadMob',                 IndepVarComp('seaSpreadMob', 0.0), promotes=['*']) #4500000.0
        self.add('compRacks',                    IndepVarComp('compRacks', 0.0), promotes=['*']) #1000000.0
        self.add('cabSurveyCR',                  IndepVarComp('cabSurveyCR', 0.0), promotes=['*']) #240.0
        self.add('cabDrillDist',                 IndepVarComp('cabDrillDist', 0.0), promotes=['*']) #500.0
        self.add('cabDrillCR',                   IndepVarComp('cabDrillCR', 0.0), promotes=['*']) #3200.0
        self.add('mpvRentalDR',                  IndepVarComp('mpvRentalDR', 0.0), promotes=['*']) #72000.0
        self.add('diveTeamDR',                   IndepVarComp('diveTeamDR', 0.0), promotes=['*']) #3200.0
        self.add('winchDR',                      IndepVarComp('winchDR', 0.0), promotes=['*']) #1000.0
        self.add('civilWork',                    IndepVarComp('civilWork', 0.0), promotes=['*']) #40000.0
        self.add('elecWork',                     IndepVarComp('elecWork', 0.0), promotes=['*']) #25000.0
        #Port & Staging', , 
        self.add('nCrane600',                    IndepVarComp('nCrane600', 0, pass_by_obj=True), promotes=['*']) #0
        self.add('nCrane1000',                   IndepVarComp('nCrane1000', 0, pass_by_obj=True), promotes=['*']) #0
        self.add('crane600DR',                   IndepVarComp('crane600DR', 0.0), promotes=['*']) #5000.0
        self.add('crane1000DR',                  IndepVarComp('crane1000DR', 0.0), promotes=['*']) #8000.0
        self.add('craneMobDemob',                IndepVarComp('craneMobDemob', 0.0), promotes=['*']) #150000.0
        self.add('entranceExitRate',             IndepVarComp('entranceExitRate', 0.0), promotes=['*']) #0.525
        self.add('dockRate',                     IndepVarComp('dockRate', 0.0), promotes=['*']) #3000.0
        self.add('wharfRate',                    IndepVarComp('wharfRate', 0.0), promotes=['*']) #2.75
        self.add('laydownCR',                    IndepVarComp('laydownCR', 0.0), promotes=['*']) #0.25
        #Engineering & Management', , 
        self.add('estEnMFac',                    IndepVarComp('estEnMFac', 0.0), promotes=['*']) #0.04
        #Development', , 
        self.add('preFEEDStudy',                 IndepVarComp('preFEEDStudy', 0.0), promotes=['*']) #5000000.0
        self.add('feedStudy',                    IndepVarComp('feedStudy', 0.0), promotes=['*']) #10000000.0
        self.add('stateLease',                   IndepVarComp('stateLease', 0.0), promotes=['*']) #250000.0
        self.add('outConShelfLease',             IndepVarComp('outConShelfLease', 0.0), promotes=['*']) #1000000.0
        self.add('saPlan',                       IndepVarComp('saPlan', 0.0), promotes=['*']) #500000.0
        self.add('conOpPlan',                    IndepVarComp('conOpPlan', 0.0), promotes=['*']) #1000000.0
        self.add('nepaEisMet',                   IndepVarComp('nepaEisMet', 0.0), promotes=['*']) #2000000.0
        self.add('physResStudyMet',              IndepVarComp('physResStudyMet', 0.0), promotes=['*']) #1500000.0
        self.add('bioResStudyMet',               IndepVarComp('bioResStudyMet', 0.0), promotes=['*']) #1500000.0
        self.add('socEconStudyMet',              IndepVarComp('socEconStudyMet', 0.0), promotes=['*']) #500000.0
        self.add('navStudyMet',                  IndepVarComp('navStudyMet', 0.0), promotes=['*']) #500000.0
        self.add('nepaEisProj',                  IndepVarComp('nepaEisProj', 0.0), promotes=['*']) #5000000.0
        self.add('physResStudyProj',             IndepVarComp('physResStudyProj', 0.0), promotes=['*']) #500000.0
        self.add('bioResStudyProj',              IndepVarComp('bioResStudyProj', 0.0), promotes=['*']) #500000.0
        self.add('socEconStudyProj',             IndepVarComp('socEconStudyProj', 0.0), promotes=['*']) #200000.0
        self.add('navStudyProj',                 IndepVarComp('navStudyProj', 0.0), promotes=['*']) #250000.0
        self.add('coastZoneManAct',              IndepVarComp('coastZoneManAct', 0.0), promotes=['*']) #100000.0
        self.add('rivsnHarbsAct',                IndepVarComp('rivsnHarbsAct', 0.0), promotes=['*']) #100000.0
        self.add('cleanWatAct402',               IndepVarComp('cleanWatAct402', 0.0), promotes=['*']) #100000.0
        self.add('cleanWatAct404',               IndepVarComp('cleanWatAct404', 0.0), promotes=['*']) #100000.0
        self.add('faaPlan',                      IndepVarComp('faaPlan', 0.0), promotes=['*']) #10000.0
        self.add('endSpecAct',                   IndepVarComp('endSpecAct', 0.0), promotes=['*']) #500000.0
        self.add('marMamProtAct',                IndepVarComp('marMamProtAct', 0.0), promotes=['*']) #500000.0
        self.add('migBirdAct',                   IndepVarComp('migBirdAct', 0.0), promotes=['*']) #500000.0
        self.add('natHisPresAct',                IndepVarComp('natHisPresAct', 0.0), promotes=['*']) #250000.0
        self.add('addLocPerm',                   IndepVarComp('addLocPerm', 0.0), promotes=['*']) #200000.0
        self.add('metTowCR',                     IndepVarComp('metTowCR', 0.0), promotes=['*']) #11518.0
        self.add('decomDiscRate',                IndepVarComp('decomDiscRate', 0.0), promotes=['*']) #0.03
        
        #self.connect('ballast_cost_rate', 'ballCR')
        #self.connect('outfitting_cost_rate', 'sSteelCR')
        #self.connect('tapered_col_cost_rate', ['spStifColCR', 'spTapColCR', 'ssStifColCR'])
