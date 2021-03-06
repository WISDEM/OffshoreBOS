/*******************************************************************************************************
*  Copyright 2017 Alliance for Sustainable Energy, LLC
*
*  NOTICE: This software was developed at least in part by Alliance for Sustainable Energy, LLC
*  (�Alliance�) under Contract No. DE-AC36-08GO28308 with the U.S. Department of Energy and the U.S.
*  The Government retains for itself and others acting on its behalf a nonexclusive, paid-up,
*  irrevocable worldwide license in the software to reproduce, prepare derivative works, distribute
*  copies to the public, perform publicly and display publicly, and to permit others to do so.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted
*  provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice, the above government
*  rights notice, this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright notice, the above government
*  rights notice, this list of conditions and the following disclaimer in the documentation and/or
*  other materials provided with the distribution.
*
*  3. The entire corresponding source code of any redistribution, with or without modification, by a
*  research entity, including but not limited to any contracting manager/operator of a United States
*  National Laboratory, any institution of higher learning, and any non-profit organization, must be
*  made publicly available under this license for as long as the redistribution is made available by
*  the research entity.
*
*  4. Redistribution of this software, without modification, must refer to the software by the same
*  designation. Redistribution of a modified version of this software (i) may not refer to the modified
*  version by the same designation, or by any confusingly similar designation, and (ii) must refer to
*  the underlying software originally provided by Alliance as �System Advisor Model� or �SAM�. Except
*  to comply with the foregoing, the terms �System Advisor Model�, �SAM�, or any confusingly similar
*  designation may not be used to refer to any modified version of this software or any modified
*  version of the underlying software originally provided by Alliance without the prior written consent
*  of Alliance.
*
*  5. The name of the copyright holder, contributors, the United States Government, the United States
*  Department of Energy, or any of their employees may not be used to endorse or promote products
*  derived from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER,
*  CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR
*  EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
*  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************************************/

#include "lib_wind_obos_orig.h"

#include <iostream>
#include <cmath>
#include <math.h>
#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <fstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

using namespace std;


//*******************************************************************************************
//Offshore BOS model 'Soft Costs' Module starts here and ends at  function definition
//*******************************************************************************************

//calculate total soft costs
void wobos_orig::Soft_costs()
{
  // calculate the capital expeditures to get the plant running
  bos_capex = totAnICost + totDevCost + totElecCost + totEnMCost + totPnSCost + subTotCost;

  //calculate insurance cost during construction
  construction_insurance_cost = construction_insurance * (nTurb*turbCapEx + bos_capex);

  // Caclulate contingencies 
  total_contingency_cost = procurement_contingency*(nTurb*turbCapEx+bos_capex-totAnICost) + install_contingency*(totAnICost);

  // Compute coft cost total up until now
  soft_costs = construction_insurance_cost + commissioning + decomCost + total_contingency_cost;
  
  // calculate construction financing cost
  construction_finance_cost = (construction_finance_factor-1)*(nTurb*turbCapEx + bos_capex + soft_costs);

  // Add in financing cost
  soft_costs += construction_finance_cost;
}

//calculate construction finance factor
void wobos_orig::Construction_finance_factor()
{
    construction_finance_factor = capital_cost_year_0*(1+(1-tax_rate)*(pow((1+interest_during_construction),(capital_cost_year_0+0.5))-1))
            +capital_cost_year_1*(1+(1-tax_rate)*(pow((1+interest_during_construction),(capital_cost_year_1+0.5))-1))
            +capital_cost_year_2*(1+(1-tax_rate)*(pow((1+interest_during_construction),(capital_cost_year_2+0.5))-1))
            +capital_cost_year_3*(1+(1-tax_rate)*(pow((1+interest_during_construction),(capital_cost_year_3+0.5))-1))
            +capital_cost_year_4*(1+(1-tax_rate)*(pow((1+interest_during_construction),(capital_cost_year_4+0.5))-1))
            +capital_cost_year_5*(1+(1-tax_rate)*(pow((1+interest_during_construction),(capital_cost_year_5+0.5))-1));
}

//calculate the present value in dollars of the cost of decommissioning the wind plant
double wobos_orig::DecomissExpense()
{
        //check if floating substructure type is selected
        if ((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
        {
                return (((0.2*(moorTime / totInstTime) + 0.6*(turbInstTime / totInstTime) + 0.1
                        *(arrInstTime / totInstTime) + 0.1*(expInstTime / totInstTime) +
                        0.4*(subsInstTime / totInstTime))*totAnICost) - scrapVal) / pow(
                        (1 + decomDiscRate), projLife);
        }
        else
        {
                return (((0.9*(subInstTime / totInstTime) + 0.7*(turbInstTime / totInstTime)
                        + 0.2*(arrInstTime / totInstTime) + 0.2*(expInstTime / totInstTime)
                        + 0.8*(subsInstTime / totInstTime))*totAnICost) - scrapVal) / pow(
                        (1 + decomDiscRate), projLife);
        }
}

//calculate total BOS costs
void wobos_orig::Total_bos_cost()
{
    total_bos_cost = totAnICost + totDevCost + totElecCost + totEnMCost + totPnSCost + subTotCost + soft_costs;
}

//*******************************************************************************************
//Offshore BOS model 'General' Module starts here and ends at TowerMass() function definition
//*******************************************************************************************

//calculate turbine hub Diameter
double wobos_orig::HubDiameter()
{
	return (turbR / 4) + 2;
}

//calculate turbine blade length
double wobos_orig::BladeLength()
{
	return (rotorD - hubD) / 2;
}

//calculate turbine nacelle width
double wobos_orig::NacelleWidth()
{
	return hubD + 1.5;
}

//calculate turbine nacelle length
double wobos_orig::NacelleLength()
{
	return 2 * nacelleW;
}

//calculate turbine RNA mass or tower head mass
double wobos_orig::RNAMass()
{
	return 2.082*pow(turbR, 2) + 44.59*turbR + 22.48;
}

//calculate tower diameter
double wobos_orig::TowerDiameter()
{
	return turbR / 2 + 4;
}

//calculate tower mass
double wobos_orig::TowerMass()
{
	return (0.4*M_PI*pow((rotorD / 2), 2)*hubH - 1500) / 1000;
}

//*******************************************************************************************
//Offshore BOS model 'Substructure & Foundation' module starts here and ends after
//SubstructTotCost() function definition
//*******************************************************************************************

double wobos_orig::MonopileLength()
{
	return waterD + mpEmbedL + 5;
}

//calculate monopile single pile mass in tonnes
double wobos_orig::MonoPileMass()
{
	return (pow((turbR * 1000), 1.5) + (pow(hubH, 3.7) / 10) + 2100 *
		pow(waterD, 2.25) + pow((rnaM * 1000), 1.13)) / 10000;
}

// calculate monopile single pile cost in dollars
double wobos_orig::MonoPileCost()
{
	return mpileM*mpileCR;
}

//calculate monopile single transition piece mass in tonnes
double wobos_orig::MonoTransMass()
{
	return exp(2.77 + 1.04*pow(turbR, 0.5) + 0.00127*pow(waterD, 1.5));
}

//calculate monopile single transition piece cost in dollars
double wobos_orig::MonoTransCost()
{
	return mtransM*mtransCR;
}

//calculate single jacket lattice mass in tonnes
double wobos_orig::JackLatticeMass()
{
	return exp(3.71 + 0.00176*pow(turbR, 2.5) + 0.645*log(waterD));
}

//calculate single jacket lattice cost in dollars
double wobos_orig::JackLatticeCost()
{
	return jlatticeM*jlatticeCR;
}

// calculate single jacket transition piece mass in tonnes
double wobos_orig::JackTransMass()
{
	return 1 / (-0.0131 + 0.0381 / log(turbR) - 0.00000000227*pow(waterD, 3));
}

// calculate single jacket transition piece cost in dollars
double wobos_orig::JackTransCost()
{
	return jtransM*jtransCR;
}

//calculate jacket pile mass in tonnes (total for 4 piles)
double wobos_orig::JackPileMass()
{
	return 8 * pow(jlatticeM, 0.5574);
}

//calculate total cost for 4 jacket piles in dollars
double wobos_orig::JackPileCost()
{
	return jpileM*jpileCR;
}

//calculate total substructure mass for a single substructure depending on substructure type
//in tonnes
double wobos_orig::SubstructTotalMass()
{
	switch (substructure)//check substructure type and calculate accordingly
	{
	case JACKET:
		return jlatticeM + jtransM + jpileM + sSteelM;
		break;
	case SPAR:
		return spTapColM + spStifColM + sSteelM + ballM;
		break;
	case SEMISUBMERSIBLE:
		return ssStifColM + ssHeaveM + ssTrussM + sSteelM;
		break;
	default:
		return mpileM + mtransM + sSteelM;
	}
}

//calculate mass of the stiffened column for single spar in tonnes
double wobos_orig::SparStifColMass()
{
	return 535.93 + 17.664*pow(turbR, 2) + 0.02328*waterD*log(waterD);
}

//calculate mass of the tapered column for a single spar in tonnes
double wobos_orig::SparTapColMass()
{
	return 125.81*log(turbR) + 58.712;
}

//calculate the ballast mass for a single spar in tonnes
double wobos_orig::BallMass()
{
	return -16.536*pow(turbR, 2) + 1261.8*turbR - 1554.6;
}

//calculate the stiffened column cost for a single spar in dollars
double wobos_orig::SparStifColCost()
{
	return spStifColM*spStifColCR;
}

//calculate the tapered column cost for a single spar in dollars
double wobos_orig::SparTapColCost()
{
	return spTapColM*spTapColCR;
}

//calculate the ballast cost for a single spar in dollars
double wobos_orig::BallCost()
{
	return ballM*ballCR;
}

//calculate the stiffened column mass for a single semisubmersible in tonnes
double wobos_orig::SemiStifColMass()
{
	return -0.9571*pow(turbR, 2) + 40.89*turbR + 802.09;
}

//calculate the truss mass for a single semisubmersible in tonnes
double wobos_orig::SemiTrussMass()
{
	return 2.7894*pow(turbR, 2) + 15.591*turbR + 266.03;
}

//calculate the heave plate mass for a single semisubmersible in tonnes
double wobos_orig::SemiHeaveMass()
{
	return -0.4397*pow(turbR, 2) + 21.545*turbR + 177.42;
}

//calculate the stiffened column cost for a single semisubmersible in dollars
double wobos_orig::SemiStifColCost()
{
	return ssStifColM*ssStifColCR;
}

//calculate the truss cost for a single semisubmersible in dollars
double wobos_orig::SemiTrussCost()
{
	return ssTrussM*ssTrussCR;
}

//calculate the heave plate cost for a single semisubmersible in dollars
double wobos_orig::SemiHeaveCost()
{
	return ssHeaveM*ssHeaveCR;
}

//calculate the mooring system and anchor cost in dollars for a singe floating substructure (spar or semisubmersible)
double wobos_orig::MooringSys()
{
	double moorLeng;//stores the length of the mooring

	if (moorDia <= 0)//check if mooring diameter was given as input
	{
		//the following if-else section selects required standard mooring sizes base on turbine rating
		if (-0.0004*pow(turbR, 2) + 0.0132*turbR + 0.0536 <= 0.09)
		{
			moorDia = 0.09;
		}
		else if (-0.0004*pow(turbR, 2) + 0.0132*turbR + 0.0536 <= 0.12)
		{
			moorDia = 0.12;
		}
		else
		{
			moorDia = 0.15;
		}
	}
	//calculate mooring line length depending on anchor type
	if (anchor == DRAGEMBEDMENT)
	{
		moorLeng = moorLines*((0.0002*pow(waterD, 2) + 1.264*waterD + 47.776) + deaFixLeng);
	}
	else //(anchor == SUCTIONPILE)
	{
		moorLeng = moorLines*(0.0002*pow(waterD, 2) + 1.264*waterD + 47.776);
	}

	double moorBL = 419449 * pow(moorDia, 2) + 93415 * moorDia - 3577.9;//calculate mooring line breaking load
	double anchorCost;
	//calculate anchor cost based on anchor type
	if (anchor == SUCTIONPILE)
	{
		anchorCost = moorLines*(sqrt(moorBL / 9.806 / 1250) * 150000);
	}
	else //(anchor == DRAGEMBEDMENT)
	{
		anchorCost = moorLines*(moorBL / 9.806 / 20 * 2000);
	}
	//select appropriate mooring line cost factor depending on the line diameter
	if (moorDia == 0.12)
	{
		moorCR = 721;
	}
	else if (moorDia == 0.15)
	{
		moorCR = 1088;
	}
	else
	{
		moorCR = 399;
	}
	double moorSysCost = anchorCost + moorLeng*moorCR;

	return moorSysCost;
}

// calculate the secondary steel mass in tonnes for a single substructure (ladders, boat landings, railing, etc.)
double wobos_orig::SecondarySteelMass()
{
	//calculate secondary steel mass for selected substructure type
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		if (turbR <= 4)//calculate secondary steel mass depending on turbine rating
		{
			return 35 + (0.8*(18 + waterD));
		}
		else
		{
			return 40 + (0.8*(18 + waterD));
		}
	}
	else if (substructure == SPAR)
	{
		return exp(3.58 + 0.196*pow(turbR, 0.5)*log(turbR) + 0.00001*waterD*log(waterD));
	}
	else
	{
		return -0.153*pow(turbR, 2) + 6.54*turbR + 128.34;
	}
}

//calculate the secondary steel cost for a single substructure in dollars
double wobos_orig::SecondarySteelCost()
{
	return sSteelM*sSteelCR;
}

//calculate the total substructure cost for entire project in dollars
double wobos_orig::SubstructTotCost()
{
	//calculate total cost depending on selected substructure type
	switch (substructure)
	{
	case JACKET:
		return (jLatticeCost + jTransCost + jPileCost + sSteelCost)*nTurb;
		break;
	case SPAR:
		return (spStifColCost + spTapColCost + ballCost + moorSysCost + sSteelCost)*nTurb;
		break;
	case SEMISUBMERSIBLE:
		return (ssStifColCost + ssHeaveCost + ssTrussCost + moorSysCost + sSteelCost)*nTurb;
		break;
	default://monopile is default
		return (mPileCost + mTransCost + sSteelCost)*nTurb;
		break;
	}
}

//*******************************************************************************************
//Offshore BOS model 'Electrical Infrastructure' module starts here and ends after
//TotElectricalCost() function definition
//*******************************************************************************************

//calculate the total number of full strings (string = a set of turbines that share the
//same electrical line back to the substation from the array)
double wobos_orig::Strings(double& cab2CurrRating, double& arrVoltage)
{
	return floor(nTurb / ((floor(((sqrt(3)*cab2CurrRating*arrVoltage*pwrFac*
		(1 - (buryDepth - 1)*buryFac)) / 1000) / turbR))));
}

//calculate the number of turbines on a partial string (partial string = a string that
//is created when the remainder of (number of turbines)/(full strings) is greater than
//zero
double wobos_orig::NumTurbParStr(double& cab2CurrRating, double& arrVoltage)
{
	return fmod((nTurb), floor(((sqrt(3)*cab2CurrRating*arrVoltage*pwrFac*
		(1 - (buryDepth - 1)*buryFac)) / 1000) / turbR));
}

//calculate the number of turbines that can fit onto array cable 1 given power transfer
//limits
double wobos_orig::NumTurbCable1(double& cab1CurrRating, double& arrVoltage)
{
	return (floor(((sqrt(3)*cab1CurrRating*arrVoltage*pwrFac*
		(1 - (buryDepth - 1)*buryFac)) / 1000) / turbR));
}

//calculate the number of turbines that can fit onto array cable 2 given power transfer
//limits
double wobos_orig::NumTurbCable2(double& cab2CurrRating, double& arrVoltage)
{
	return (floor(((sqrt(3)*cab2CurrRating*arrVoltage*pwrFac*
		(1 - (buryDepth - 1)*buryFac)) / 1000) / turbR));
}

//calculate the number of turbine interfaces on array cable 1
double wobos_orig::InterfacesCable1(double& fullStrings, double& nTurbPS, double& nTurbCab1)
{
	if (nTurbPS == 0)//check if any partial strings exist
	{
		return (nTurbCab1*fullStrings) * 2;
	}
	else
	{
		return (nTurbCab1*fullStrings + min((nTurbPS - 1), nTurbCab1)) * 2;
	}
}

//calculate the number of turbine interfaces on array cable 2
double wobos_orig::InterfacesCable2(double& fullStrings, double& nTurbPS, double&nTurbCab1, double& nTurbCab2)
{
	//calculate maximum values and store in 'a' and 'b'
	double max1 = nTurbCab2 - nTurbCab1;
    if(max1 <= 0) max1 = 0;
	double max2 = nTurbPS - nTurbCab1 - 1;
    if(max2 <= 0) max2 = 0;

	if (nTurbPS == 0)//check if any partial strings exist
	{
		return (max1*fullStrings + max2) * 2;
	}
	else
	{
		return ((max1*fullStrings + max2) * 2) + 1;
	}
}

//calculate the number of array cable substation interfaces
//array cable 2 is used for all runs from array to substation
double wobos_orig::SubstationInterfaces(double& fullStrings, double& nTurbPS)
{
	if (nTurbPS == 0)//check if any partial strings exist
	{
		return fullStrings;
	}
	else
	{
		return fullStrings + 1;
	}
}

//calculate system angle (system angle = a value used to calculate a
//hypotenuse distance which is used to approximate the free hanging
//length of the array cable for floating wind plants)
double wobos_orig::SystemAngle()
{
	return -0.0047*waterD + 18.743;
}

// calculate the free hanging or catenary length in meters of cable that hangs from floating
//turbines down to the sea floor
double wobos_orig::FreeCable()
{
	return (waterD / cos(systAngle*(M_PI / 180)))*(catLengFac + 1) + 190;
}

//Calculate the fixed cable length in meters along the sea floor between the free hanging sections
//located in between turbine interfaces
double wobos_orig::FixedCable()
{
	return (arrayY*rotorD) - (2 * (((tan(systAngle*(M_PI / 180)))*waterD) + 70));
}

//calculate the length of array cable 1
double wobos_orig::Cable1Length(double& nTurbInter1)
{
	//check if floating or fixed substructure type
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		return (arrayY*rotorD + waterD * 2)*(nTurbInter1 / 2)*(1 + exCabFac);
	}
	else //((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
	{
		return (2 * freeCabLeng + fixCabLeng)*(nTurbInter1 / 2)*(1 + exCabFac);
	}
}

//calculate the length of array cable 2 in meters
double wobos_orig::Cable2Length(double& nTurbCab1, double& nTurbCab2, double& fullStrings, double& nTurbPS)
{
	//calculate maximum values and store in 'max1' and 'max2'
	double max1 = nTurbCab2 - nTurbCab1 -1;
    if(max1 <= 0) max1 = 0;
	double max2 = nTurbPS - nTurbCab1 - 1;
    if(max2 <= 0) max2 = 0;

	double stringFac;
	//'stringFac' changes depending on if a partial string exists
	if (nSubstation > 0)//protect against division by zero if 'nSubstation' is zero
	{
		if (nTurbPS == 0)
		{
			stringFac = (fullStrings / nSubstation);
		}
		else
		{
			stringFac = (fullStrings + 1) / nSubstation;
		}
	}
	else
	{
		if (nTurbPS == 0)
		{
			stringFac = fullStrings;
		}
		else
		{
			stringFac = (fullStrings + 1);
		}
	}
	
	//check if substructure is floating or fixed type
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		return (((arrayY*rotorD + waterD * 2)*(max1*fullStrings + max2)) + nSubstation*((stringFac*
			((rotorD*arrayY) + sqrt(pow(((rotorD*arrayX)*(stringFac - 1)), 2) +
			pow((rotorD*arrayY), 2)))) / 2 + stringFac*waterD))*(exCabFac + 1);
	}
	else //((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
	{
		return (((2 * freeCabLeng + fixCabLeng)*max1*fullStrings + max2) + nSubstation
			*(stringFac*((2 * freeCabLeng + fixCabLeng) + sqrt(pow(((stringFac - 1)
			*((2 * freeCabLeng) + (arrayX*rotorD) - (2 * ((tan(systAngle*(M_PI / 180))*waterD) + 70)))), 2)
			+ pow((2 * freeCabLeng + fixCabLeng), 2)))) / 2)*(exCabFac + 1);
	}

}

//calculate the total number of export cables that are required based on electrical limits
//of the cables
double wobos_orig::NumberExportCable(double& expCurrRating, double& expVoltage)
{
	return ceil(((turbR*nTurb) / ((sqrt(3)*expCurrRating*expVoltage*pwrFac*(1 - (buryDepth - 1)*buryFac)) / 1000)));
}

//calculate the export cable length in meters
double wobos_orig::ExportCableLength(double& nExportCab)
{
	//check if substructure is floating or fixed type
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		return (distShore * 1000 + waterD)*nExportCab*1.1;
	}
	else //((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
	{
		return (distShore * 1000 + freeCabLeng + 500)*nExportCab*1.1;
	}
}

//calculate the number of substations that are required
double wobos_orig::NumberSubstation(double& nExportCab)
{
	if (nExportCab >= 2)
	{
		return ceil((nExportCab / 2));
	}
	else
	{
		return 1;
	}
}

//calculate the total number of main power transformers (MPTs) that are required
double wobos_orig::NumberMPT()
{
	return ceil(((nTurb*turbR) / 250));
}

//calculate the rating in megavolt amperes of a single MPT
double wobos_orig::SingleMPTRating()
{
	//if the remainder of '((nTurb*turbR*1.15)/nMPT)/10' is greater than 5 round up, else round down
	double mptRating = ceil(fmod((nTurb*turbR*1.15) / nMPT, 10)) >= 5
		? ceil(((nTurb*turbR*1.15) / nMPT) / 10) * 10 :
		floor(((nTurb*turbR*1.15) / nMPT) / 10) * 10;

	return mptRating;
}

//calculate the total cost for all MPTs in dollars
double wobos_orig::MPTCost()
{
	return mptRating*nMPT*mptCR;
}

//calculate the topside mass of the offshore substation(s) in tonnes
double wobos_orig::SubstationTopsideMass()
{
	return 3.85*(mptRating*nMPT) + 285;
}

//calculate the substructure mass of the offshore substation substructure mass in tonnes
double wobos_orig::SubstationSubMass()
{
	//check if floating substructure type is selected
	if ((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
	{
		return 2 * (ssStifColM + ssTrussM + ssHeaveM + sSteelM);
	}
	else
	{
		return 0.4*subsTopM;
	}
}

//calculate the mass of the jacket piles in tonnes used for offshore substation substructure
//for fixed turbine wind plants
double wobos_orig::SubstationSubPileMass()
{
	//check if fixed substructure type is selected
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		return 8 * pow(subsSubM, 0.5574);
	}
	else
	{
		return 0;
	}
}

//calculate the total substructure cost for the offshore substation
double wobos_orig::SubstationSubCost()
{
	//check if floating substructure type is selected
	if ((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
	{
		return 2 * (ssTrussCost + ssStifColCost + ssHeaveCost + moorSysCost);
	}
	else
	{
		return subsSubM*subsJackCR + subsPileCR*subsPileM;
	}
}

//calculate the offshore substation topside cost in dollars
double wobos_orig::SubstationTopsideCost()
{
	return subsTopM*subsTopFab + subsTopDes;
}

//calculate the cost of assembling the offshore substation on land in dollars
double wobos_orig::LandTopsideAssembly()
{
	return (switchGear + shuntReactors + mptCost)*topAssemblyFac;
}

//calculate the onshore substation cost in dollars
double wobos_orig::OnshoreSubsCost()
{
	return 11652 * (interConVolt + turbR*nTurb) + 1200000;
}

//calculate miscellaneous costs associated with the onshore substation in dollars
double wobos_orig::OnshoreSubsMiscCost()
{
	return 11795 * pow((turbR*nTurb), 0.3549) + 350000;
}

//calculate the cost in dollars of the overhead transmission line for connection back to grid
double wobos_orig::TransLineCost()
{
	//check distance to interconnect for zero value
	if (distInterCon == 0)
	{
		return 0;
	}
	else
	{
		return (1176 * interConVolt + 218257)*pow(distInterCon, -0.1063)*distInterCon;
	}
}

//calculate the cost in dollars of the electrical switch yard
double wobos_orig::SwitchYardCost()
{
	return 18115 * interConVolt + 165944;
}

//calculate the total cost in dollars of array cable 1 including interface costs
double wobos_orig::ArrayCable1Cost(double& cab1Leng, double& cab1CR, double& turbInterCR1,
	double& nTurbInter1)
{
	//check if floating substructure type is selected
	if ((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
	{
		return dynCabFac*cab1Leng*cab1CR + turbInterCR1*nTurbInter1;
	}
	else
	{
		return cab1Leng*cab1CR + turbInterCR1*nTurbInter1;
	}
}

//calculate the total cost in dollars of array cable 2 including interface costs
double wobos_orig::ArrayCable2Cost(double& cab2Leng, double& cab2CR, double& turbInterCR2,
	double& nTurbInter2, double& nSubsInter, double& arrSubsInterCR)
{
	//check if floating substructure type is selected
	if ((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
	{
		return dynCabFac*cab2Leng*cab2CR + nTurbInter2*turbInterCR2 +
			nSubsInter*arrSubsInterCR;
	}
	else
	{
		return cab2Leng*cab2CR + nTurbInter2*turbInterCR2 +
			nSubsInter*arrSubsInterCR;
	}
}

//calculate the total cost in dollars of the export cabling including interface costs
double wobos_orig::ExportCableCost(double& expSubsInterCR, double& expCabCR, double& expCabLeng, double& nExpCab)
{
	//check if floating substructure type is selected
	if ((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
	{
		return expCabCR*((expCabLeng - 500 - freeCabLeng) + dynCabFac*(500 + freeCabLeng))
			+ expSubsInterCR*nExpCab;
	}
	else
	{
		return expCabCR*expCabLeng + expSubsInterCR*nExpCab;
	}

}

//calculate the cost in dollars of the switchgear used for redundancy and protection
//against electrical surges or faults
double wobos_orig::Switchgear()
{
	return nMPT*(highVoltSG + medVoltSG);
}

//calculate the cost in dollars of the shunt reactors used to dissipate capacitive
//reactance
double wobos_orig::ShuntReactors()
{
	return mptRating*nMPT*shuntCR*0.5;
}

//calculate the cost in dollars of any remaining, necessary ancillary systems
double wobos_orig::AncillarySystems()
{
	return backUpGen + workSpace + otherAncillary;
}

//calculate the total cost in dollars of the sub-sea cabling which includes export
//and array cabling
double wobos_orig::SubseaCableCost()
{
	return arrCab1Cost + arrCab2Cost + expCabCost;
}

//calculate the total cost in dollars of the offshore substation
double wobos_orig::OffshoreSubstationCost()
{
	return (subsTopCost + switchGear + shuntReactors + ancillarySys + mptCost + subsLandAssembly + subsSubCost)
		*nSubstation;
}

//calculate the total cost in dollars of the onshore transmission system which includes the onshore
//substation, switch yard, connection to grid, and other misc. costs
double wobos_orig::OnshoreTransCost()
{
	return onShoreSubs + onshoreMisc + transLine + switchYard;
}

//calculate the total cost for the entire electrical infrastructure system
double wobos_orig::TotElectricalCost()
{
	return subCabCost + offSubsCost + onshoreTransCost;
}

//*******************************************************************************************
//Offshore BOS model 'Assembly & Installation' module starts here and ends at TotInstCost()
//function definition
//*******************************************************************************************

//calculate the total duration in days for the mooring system installation
double wobos_orig::MooringSysInstall()
{
	double moorTime;
	//check for anchor type
	if (anchor == DRAGEMBEDMENT)
	{
		moorTime = 5 + waterD*moorTimeFac;
	}
	else // (anchor == SUCTIONPILE)
	{
		moorTime = 11 + waterD*moorTimeFac;
	}

	return ceil((((moorLoadout + moorSurvey + moorTime)*moorLines + (waterD*moorTimeFac)*moorLines +
		(distPort * 1000 * 2 / (subInstVessel[11] * 1852)))*nTurb / 24)*(1 / (1 - substructCont)));
}

//calculate the total duration in days to prep floating substructures for turbine installation
double wobos_orig::PrepFloatSubstructure()
{
	//check if spar substructure is selected
	if (substructure == SPAR)
	{
		return ceil(((((prepSpar + upendSpar) + (distPtoA / subInstVessel[21]))*nTurb) / 24) + prepAA / 24);
	}
	else
	{
		return ceil((prepSemi*nTurb) / 24);
	}
}

//calculate the minimum deck area that is required to for a single turbine
double wobos_orig::MinTurbDeckArea()
{
	double area1;
	double area2;
	//check for turbine installation method
	switch (turbInstallMethod)
	{
	case ROTORASSEMBLED:
		area1 = (bladeL + inspectClear)*(chord + inspectClear) + (M_PI*pow((hubD / 2), 2)) / 2
			+ (nacelleL + inspectClear)*(nacelleW + inspectClear);
		break;
	case BUNNYEARS:
		area1 = (bladeL + inspectClear)*(chord + inspectClear) + (nacelleL + inspectClear)
			*(nacelleW / 2 + bladeL / 2 * 1.73 + inspectClear);
		break;
	default:
		area1 = (bladeL + inspectClear)*(chord + inspectClear) + (nacelleL + inspectClear)
			*(nacelleW + inspectClear);
		break;
	}
	//check for tower installation method
	switch (towerInstallMethod)
	{
	case ONEPIECE:
		area2 = area1 + pow((towerD + inspectClear), 2);
		break;
	default:
		area2 = area1 + pow((towerD + inspectClear), 2) * 2;
		break;
	}

	return area2;
}

//calculated the maximum number of turbines that can be transported to the install site
double wobos_orig::TurbsPerTrip()
{
	//check for substructure type
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		if (installStrategy == PRIMARYVESSEL)
		{
			return floor(min((turbInstVessel[7] / turbDeckArea), (turbInstVessel[8]
				/ (rnaM + towerM))));
		}
		else
		{
			return floor(min((turbFeederBarge[7] / turbDeckArea), (turbFeederBarge[8]
				/ (rnaM + towerM))));
		}
	}
	else if (substructure == SPAR)
	{
		return floor(min((turbFeederBarge[7] / turbDeckArea), (turbFeederBarge[8]
			/ (rnaM + towerM))));
	}
	else
	{
		return 1;
	}
}

//calculate the minimum turbine deck area that is required for a single substructure (fixed substructures only
//return 0 for floating substructures)
double wobos_orig::MinSubDeckArea()
{
	//check substructure type
	switch (substructure)
	{
	case MONOPILE:
		return (mpileL + inspectClear)*(mpileD + inspectClear) + pow((mpileD + inspectClear + 1), 2);
		break;
	case JACKET:
		return pow((jlatticeA + inspectClear), 2) + (jpileD + inspectClear)*(jpileL + inspectClear);
		break;
	default:
		return 0;
		break;
	}
}

//calculate the maximum number of substructures that can be transported to the install site
double wobos_orig::SubPerTrip()
{
	//check if fixed substructure type is selected
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		if (installStrategy == PRIMARYVESSEL)
		{
			return floor(min((subInstVessel[7] / subDeckArea), (subInstVessel[8] / subTotM)));
		}
		else
		{
			return floor(min((subFeederBarge[7] / subDeckArea), (subFeederBarge[8] / subTotM)));
		}
	}
	else
		return 0;
}

//calculate the total duration in days required to install all turbines
double wobos_orig::TurbineInstall()
{
	double sum;
	//check turbine installation method
	switch (turbInstallMethod)
	{
	case ROTORASSEMBLED:
		sum = vesselPosTurb + boltTower + boltNacelle3 + boltRotor;
		break;
	case BUNNYEARS:
		sum = vesselPosTurb + boltTower + boltNacelle2 + boltBlade2;
		break;
	default:
		sum = vesselPosTurb + boltTower + boltNacelle1 + 3 * boltBlade1;
		break;
	}
	//check tower installation method
	if (towerInstallMethod == TWOPIECE)
	{
		sum += boltTower;
	}
	//check if floating substructure is selected
	if ((substructure == SEMISUBMERSIBLE) || (substructure == SPAR))
	{
		sum -= vesselPosTurb + turbFasten;
	}

	double sum2 = 0;
	//check installation vessel strategy
	if (installStrategy == PRIMARYVESSEL)
	{
		sum2 = (ceil(nTurb / nTurbPerTrip))*(distPort / ((turbInstVessel[11] * 1852) / 1000)) * 2 + turbFasten*nTurb;
	}
	if (nTurbPerTrip <= 0)//ensures turbines per trip is greater than or equal 1
	{
		nTurbPerTrip = 1;
	}
	//check substructure type
	if (substructure == SPAR)
	{
		return ceil(1.5*(((distPtoA / (turbInstVessel[21]))*(nTurb / nTurbPerTrip)) / 24 + sum
			*(1 / (1 - turbCont))*nTurb / 24) + (1 / (1 - substructCont))*(nTurb / 24)*(distAtoS /
			(turbInstVessel[21]) + (spMoorCon + spMoorCheck + prepTow)));

	}
	else if (substructure == SEMISUBMERSIBLE)
	{
		return ceil(sum*(1 / (1 - turbCont))*nTurb / 24 + ((nTurb / 24)*(1 / (1 - substructCont))*
			((prepTow + ssBall + ssMoorCheck + ssMoorCon) + (distPort / turbInstVessel[21]))));
	}
	else // ((substructure == MONOPILE) || (substructure == JACKET))
	{
		return ceil((((sum + ((waterD + 10) / (turbInstVessel[6] * 60)) * 2)*nTurb + (nTurb
			- ceil((nTurb / nTurbPerTrip)))*(arrayY*rotorD) / (turbInstVessel[11] * 1852)
			+ sum2) / 24)*(1 / (1 - turbCont)));
	}

}

//calculate the total duration in days required to install all substructures
double wobos_orig::SubstructureInstTime()
{
	double fac1 = 0;
        if(nSubPerTrip <= 0) //ensure that no NaNs appear from division
            nSubPerTrip = 1;

	//check installation vessel strategy
	if (installStrategy == PRIMARYVESSEL)
	{
		switch (substructure)
		{
		case JACKET:
			fac1 = ceil(nTurb / nSubPerTrip)*(distPort / (subInstVessel[11] * 1.852)) * 2 * 2
				+ 2 * jackFasten*nTurb;
			break;

		default:
			fac1 = ceil(nTurb / nSubPerTrip)*(distPort / (subInstVessel[11] * 1.852)) * 2 * 1
				+ monoFasten*nTurb;
			break;
		}
	}

	if (nSubPerTrip <= 0)//ensures substructures per trip is greater than or equal to 1
	{
		nSubPerTrip = 1;
	}

	//check substructure type
	double sum1;
	switch (substructure)
	{
	case JACKET:
		sum1 = vesselPosJack * 2 + placeTemplate + prepGripperJack + placePiles + prepHamJack + removeHamJack + placeJack
			+ levJack + ((jpileL - 5) / hamRate) * 4 + ((waterD + 10) / (subInstVessel[6] * 60)) * 2 * 2;
		break;
	default:
		sum1 = vesselPosMono + prepGripperMono + placeMP + prepHamMono + removeHamMono + placeTP + groutTP //change
			+ tpCover + (mpEmbedL / hamRate) + ((waterD + 10) / (subInstVessel[6] * 60)) * 2;
		break;
	}
	//check if fixed substructure type is selected
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		double x = ceil(((1 / (1 - substructCont))*((sum1*nTurb + (nTurb - ceil((nTurb / nSubPerTrip)))
			*(rotorD*arrayX) / (subInstVessel[11] * 1852)) + fac1)) / 24);
		return x;
	}
	else
	{
		return moorTime + floatPrepTime;
	}
}

//calculate the mass of each section that makes up array cable 1
double wobos_orig::Cab1SecMass(double& arrCab1Mass)
{
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		return arrCab1Mass*(arrayY*rotorD + waterD * 2)*(1 + exCabFac) / 1000;
	}
	else
	{
		return arrCab1Mass*(freeCabLeng * 2 + fixCabLeng)*(1 + exCabFac) / 1000;
	}
}

//calculate the mass of each section that makes up array cable 2
double wobos_orig::Cab2SecMass(double& arrCab2Mass)
{
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		return arrCab2Mass*(arrayY*rotorD + waterD * 2)*(1 + exCabFac) / 1000;
	}
	else
	{
		return arrCab2Mass*(freeCabLeng * 2 + fixCabLeng)*(1 + exCabFac) / 1000;
	}
}


//calculate the total number of cable sections per vessel trip for array cable 1
double wobos_orig::Cab1SecPerTrip(double& cab1SecM)
{
	return floor(arrCabInstVessel[22] / cab1SecM);
}

//calculate the total number of cable sections per vessel trip for array cable 2
double wobos_orig::Cab2SecPerTrip(double cab2SecM)
{
	return floor(arrCabInstVessel[22] / cab2SecM);
}

//calculate the total duration in days required to install the array cabling
double wobos_orig::ArrayCabInstTime(double& cab1Leng, double& cab2Leng, double& interCab1,
	double& interCab2, double& subsInter, double& cab1SecPerTrip,
	double& cab2SecPerTrip, double& fullStrings, double& nTurbPS,
	double& nTurbCab1, double nTurbCab2)
{
	double max1 = nTurbCab2 - nTurbCab1;
    if(max1 <= 0) max1 = 0;
	double max2 = nTurbPS - nTurbCab1 - 1;
    if(max2 <= 0) max2 = 0;

	//check if cable is buried or not
	double fac1 = 0;
	if (buryDepth > 0)
	{
		fac1 = 1 / buryRate;
	}

	//check if a partial string exists
	double fac2;
	if (nTurbPS == 0)
	{
		fac2 = (fullStrings*(fullStrings + 1)) / 2;
	}
	else
	{
		fac2 = ((fullStrings + 1)*((fullStrings + 1) + 1)) / 2;
	}

	return ceil((1 + ((((cab1Leng + cab2Leng - (waterD*(interCab1 + interCab2
		+ subsInter))*(1 + exCabFac))*(fac1 + 1 / surfLayRate) +
		(cabPullIn + cabTerm)*(interCab1 + interCab2 + subsInter))
		+ (ceil(((interCab1*0.5) / cab1SecPerTrip)) + (ceil(((max1
		*fullStrings + max2 + fac2) / cab2SecPerTrip))))*(cabLoadout
		+ distPort / (arrCabInstVessel[11] * 1.852)))*(1 / (1 - elecCont)) / 24)));

}

//calculate the mass of each section that makes up the export cable(s)
double wobos_orig::ExportCableSecMass(double& expCabMass, double& exportLeng, double& nExportCab)
{
	return expCabMass*exportLeng / nExportCab / 1000;
}

//calculate the total number of cable sections per vessel trip for the export cable(s)
double wobos_orig::ExportCabSecPerTrip(double& expCabSecM)
{
	return (expCabInstVessel[22] / expCabSecM);
}

//calculate the total duration in days required to install the export cable system
double wobos_orig::ExportCabInstallTime(double& expCabSecPerTrip, double& nExportCab)
{
	double fac = 0;
	//check if cable is buried or not
	if (buryDepth > 0)
	{
		fac = 1 / buryRate;
	}

	return ceil(ceil((ceil(nExportCab / expCabSecPerTrip)*(distPort /
		(expCabInstVessel[11] * 1.852) + expCabLoad) + (1 + exCabFac)*(distShore * 1000)*(1 / surfLayRate + fac)
		+ (subsPullIn + shorePullIn + cabTerm)*nExportCab) / 24 + landConstruct)*(1 / (1 - elecCont)));
}

//calculate the total duration in days required to install the offshore substation
double wobos_orig::SubsInstallTime()
{
	//check if fixed substructure type is selected
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		return ceil(((subsLoad + subsVessPos + placeTop) + (distPort / (substaInstVessel[11] * 1.852)))
			/ 24 * (1 / (1 - elecCont)));
	}
	else
	{
		return ceil(((subsLoad + subsVessPos + placeTop + ssMoorCon + ssMoorCheck)
			+ (distPort / (substaInstVessel[21] * 1.852))) / 24 * (1 / (1 - elecCont)));
	}
}

//calculate the total duration in days required to install the complete wind plant
double wobos_orig::TotalInstallTime()
{
	//check if floating substructure type is selected
	if ((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
	{
		return moorTime + floatPrepTime + turbInstTime + arrInstTime + expInstTime + subsInstTime;
	}
	else
	{
		return turbInstTime + subInstTime + arrInstTime + expInstTime + subsInstTime;
	}
}

//Calculate the cost of each vessel used for turbine installation
void wobos_orig::TurbInstCost()
{
	//check installStrategy
	if ((installStrategy == FEEDERBARGE) || (substructure == SPAR))
	{
		turbCostsByVessel.resize(2 + turbSupportVessels.size());//size cost vector to fit all possible vessels
		for (size_t i = 0; i < turbCostsByVessel.size(); i++)
		{
			turbCostsByVessel[i].resize(2);
		}

		//populate cost vector with vessel identifier values and costs
		turbCostsByVessel[0][0] = turbInstVessel[0];
		turbCostsByVessel[1][0] = turbFeederBarge[0];
		turbCostsByVessel[0][1] = turbInstVessel[16] * turbInstVessel[14] * turbInstTime;
		turbCostsByVessel[1][1] = turbFeederBarge[16] * turbFeederBarge[14] * turbInstTime;

		for (size_t i = 2; i < turbSupportVessels.size(); i++)
		{
			turbCostsByVessel[i][0] = turbSupportVessels[i - 2][0];
			turbCostsByVessel[i][1] = turbSupportVessels[i - 2][16] * turbSupportVessels[i - 2][14]
				* turbInstTime;
		}
	}
	else
	{
		turbCostsByVessel.resize(1 + turbSupportVessels.size());//size cost vector to fit all possible vessels
		for (size_t i = 0; i < turbCostsByVessel.size(); i++)
		{
			turbCostsByVessel[i].resize(2);
		}

		//populate cost vector with vessel identifier values and costs
		turbCostsByVessel[0][0] = turbInstVessel[0];
		turbCostsByVessel[0][1] = turbInstVessel[16] * turbInstVessel[14] * turbInstTime;

		for (size_t i = 1; i < turbCostsByVessel.size(); i++)
		{
			turbCostsByVessel[i][0] = turbSupportVessels[i - 1][0];
			turbCostsByVessel[i][1] = turbSupportVessels[i - 1][16] * turbSupportVessels[i - 1][14]
				* turbInstTime;
		}
	}
	turbine_install_cost = 0;
    for(size_t i = 0; i<turbCostsByVessel.size(); i++)
    {
        turbine_install_cost += turbCostsByVessel[i][1];
    }
}


void wobos_orig::SubInstCost()
//looks like some entries don't get filled?
{
  substructure_install_cost = 0;
  //check installStrategy
  if ((installStrategy == FEEDERBARGE) || (substructure == SPAR))
    {
      subCostsByVessel.resize(3 + subSupportVessels.size());//size cost vector to fit all possible vessels
      for (size_t i = 0; i < subCostsByVessel.size(); i++)
	{
	  subCostsByVessel[i].resize(2);
	}

      //populate vectors with vessel identifier values and costs
      subCostsByVessel[0][0] = subInstVessel[0];
      subCostsByVessel[1][0] = subFeederBarge[0];
      if(substructure == SPAR)
        {
	  subCostsByVessel[0][1] = subInstVessel[16] * subInstVessel[14] * moorTime;
	  subCostsByVessel[1][1] = subFeederBarge[16] * subFeederBarge[14] * (subInstTime-moorTime);
        }
      else
        {
	  subCostsByVessel[0][1] = subInstVessel[16] * subInstVessel[14] * subInstTime;
	  subCostsByVessel[1][1] = subFeederBarge[16] * subFeederBarge[14] * subInstTime;
        }

      if(substructure == MONOPILE)//change
        {
	  subCostsByVessel[2][0] = scourProtVessel[0];
	  subCostsByVessel[2][1] = (instScour/24)*nTurb*scourProtVessel[16]*scourProtVessel[14];
        }
      for (size_t i = 3; i < subCostsByVessel.size(); i++)
	{
	  subCostsByVessel[i][0] = subSupportVessels[i - 3][0];
	  if(substructure == SPAR)
	    subCostsByVessel[i][1] = subSupportVessels[i - 3][16] * subSupportVessels[i - 3][14]
	      * (subInstTime - moorTime);
	  else
	    subCostsByVessel[i][1] = subSupportVessels[i - 3][16] * subSupportVessels[i - 3][14]
	      * subInstTime;
	}
    }
  else
    {
      subCostsByVessel.resize(2 + subSupportVessels.size());//size cost vector to fit all possible vessels
      for (size_t i = 0; i < subCostsByVessel.size(); i++)
	{
	  subCostsByVessel[i].resize(2);
	}

      //populate vectors with vessel identifier values and costs
      subCostsByVessel[0][0] = subInstVessel[0];
      subCostsByVessel[0][1] = subInstVessel[16] * subInstVessel[14] * subInstTime;

      if(substructure == MONOPILE)//change
        {
	  subCostsByVessel[1][0] = scourProtVessel[0];
	  subCostsByVessel[1][1] = (instScour/24)*nTurb*scourProtVessel[16]*scourProtVessel[14];
        }
      for (size_t i = 2; i < subCostsByVessel.size(); i++)
	{
	  subCostsByVessel[i][0] = subSupportVessels[i - 2][0];
	  subCostsByVessel[i][1] = subSupportVessels[i - 2][16] * subSupportVessels[i - 2][14] * subInstTime;
	}
    }
  for(size_t i = 0; i<subCostsByVessel.size(); i++)
    {
      substructure_install_cost += subCostsByVessel[i][1];
    }
}


void wobos_orig::ElectricalInstCost()
//looks like some entries don't get filled?
{
	elecCostsByVessel.resize((5 + elecSupportVessels.size()));//size cost vector to fit all possible vessels
	for (size_t i = 0; i < elecCostsByVessel.size(); i++)
	{
		elecCostsByVessel[i].resize(2);
	}

	//populate vectors with vessel identifier values and costs
	elecCostsByVessel[0][0] = arrCabInstVessel[0];
	elecCostsByVessel[1][0] = expCabInstVessel[0];
	elecCostsByVessel[2][0] = substaInstVessel[0];
    elecCostsByVessel[3][0] = elecTugs[0][0];
    elecCostsByVessel[4][0] = elecTugs[1][0];
	elecCostsByVessel[0][1] = arrCabInstVessel[14] * arrCabInstVessel[16] * arrInstTime;
	elecCostsByVessel[1][1] = expCabInstVessel[14] * expCabInstVessel[16] * expInstTime;

    array_cable_install_cost = elecCostsByVessel[0][1];
    export_cable_install_cost = elecCostsByVessel[1][1];

	if(substructure == MONOPILE || substructure == JACKET)
    {
	elecCostsByVessel[2][1] = substaInstVessel[14] * substaInstVessel[16] * subsInstTime;
    }
    elecCostsByVessel[3][1] = elecTugs[0][14] * elecTugs[0][16] * subsInstTime;
    elecCostsByVessel[4][1] = elecTugs[1][14] * elecTugs[1][16] * subsInstTime;
	for (size_t i = 5; i < elecCostsByVessel.size(); i++)
	{
		elecCostsByVessel[i][0] = elecSupportVessels[i - 5][0];
		elecCostsByVessel[i][1] = elecSupportVessels[i - 5][14] * elecSupportVessels[i - 5][16]
			* (arrInstTime + expInstTime + subsInstTime);
	}
	electrical_install_cost = 0;
	substation_install_cost = elecCostsByVessel[2][1]+elecCostsByVessel[3][1]+elecCostsByVessel[4][1];
    for(size_t i = 0; i<elecCostsByVessel.size(); i++)
    {
        electrical_install_cost += elecCostsByVessel[i][1];
    }
}


void wobos_orig::VesselMobDemobCost()
//looks like some entries don't get filled?
{
	//size cost vector to fit all support vessels for entire installation
	mobDemobCostByVessel.resize(subSupportVessels.size() + turbSupportVessels.size() + elecSupportVessels.size() + 10);
	for (size_t i = 0; i < mobDemobCostByVessel.size(); i++)
	{
		mobDemobCostByVessel[i].resize(2);
	}
	mobDemobCostByVessel[0][0] = turbInstVessel[0];
	mobDemobCostByVessel[1][0] = subInstVessel[0];
	mobDemobCostByVessel[2][0] = arrCabInstVessel[0];
	mobDemobCostByVessel[3][0] = expCabInstVessel[0];
	mobDemobCostByVessel[4][0] = substaInstVessel[0];
    mobDemobCostByVessel[5][0] = scourProtVessel[0];
    mobDemobCostByVessel[6][0] = elecTugs[0][0];
    mobDemobCostByVessel[7][0] = elecTugs[1][0];
	mobDemobCostByVessel[0][1] = turbInstVessel[14] * turbInstVessel[15] * turbInstVessel[16];
	mobDemobCostByVessel[1][1] = subInstVessel[14] * subInstVessel[15] * subInstVessel[16];
	mobDemobCostByVessel[2][1] = arrCabInstVessel[14] * arrCabInstVessel[15] * arrCabInstVessel[16];
	mobDemobCostByVessel[3][1] = expCabInstVessel[14] * expCabInstVessel[15] * expCabInstVessel[16];
	mobDemobCostByVessel[4][1] = substaInstVessel[14] * substaInstVessel[15] * substaInstVessel[16];
    mobDemobCostByVessel[5][1] = scourProtVessel[14] * scourProtVessel[15] * scourProtVessel[16];
    mobDemobCostByVessel[6][1] = elecTugs[0][14] * elecTugs[0][15] * elecTugs[0][16];
    mobDemobCostByVessel[7][1] = elecTugs[1][14] * elecTugs[1][15] * elecTugs[1][16];
    if(installStrategy == FEEDERBARGE || substructure == SPAR)
    {
        mobDemobCostByVessel[8][0] = turbFeederBarge[0];
        mobDemobCostByVessel[9][0] = subFeederBarge[0];
        mobDemobCostByVessel[8][1] = turbFeederBarge[14] * turbFeederBarge[15] * turbFeederBarge[16];
        mobDemobCostByVessel[9][1] = subFeederBarge[14] * subFeederBarge[15] * subFeederBarge[16];
    }
	//populate cost vector with support vessel identifier values and costs
	for (size_t i = 10; i < turbSupportVessels.size(); i++)
	{
		mobDemobCostByVessel[i][0] = turbSupportVessels[i-10][0];
		mobDemobCostByVessel[i][1] = turbSupportVessels[i-10][14] * turbSupportVessels[i-10][15] * turbSupportVessels[i-10][16];
	}
	//populate cost vector with support vessel identifier values and costs
	for (size_t i = 10 + turbSupportVessels.size(); i < turbSupportVessels.size() + subSupportVessels.size(); i++)
	{
		mobDemobCostByVessel[i][0] = subSupportVessels[i - turbSupportVessels.size()-10][0];
		mobDemobCostByVessel[i][1] = subSupportVessels[i - turbSupportVessels.size()-10][14] * subSupportVessels[i - turbSupportVessels.size()-10][15]
			* subSupportVessels[i - turbSupportVessels.size()-10][16];
	}
	//populate cost vector with support vessel identifier values and costs
	for (size_t i = 10 + turbSupportVessels.size() + subSupportVessels.size(); i < turbSupportVessels.size() + subSupportVessels.size()
		+ elecSupportVessels.size(); i++)
	{
		mobDemobCostByVessel[i][0] = elecSupportVessels[i - (turbSupportVessels.size() + subSupportVessels.size())-10][0];
		mobDemobCostByVessel[i][1] = elecSupportVessels[i - (turbSupportVessels.size() + subSupportVessels.size())-10][14]
			* elecSupportVessels[i - (turbSupportVessels.size() + subSupportVessels.size())-10][15]
			* elecSupportVessels[i - (turbSupportVessels.size() + subSupportVessels.size())-10][16];
	}

	//create an iterator to iterate through vessel identifier values
	//and place duplicates at the end of the vector
	vector<vector<double> >::iterator it;
    sort(mobDemobCostByVessel.begin(),mobDemobCostByVessel.end());
	it = unique(mobDemobCostByVessel.begin(), mobDemobCostByVessel.end());

	//resize the cost vector to get rid of the duplicate support vessels stored at the end of the vector
	int resizer = (int)distance(mobDemobCostByVessel.begin(), it);
	mobDemobCostByVessel.resize(resizer);
	for (int i = 0; i < resizer; i++)
	{
		mobDemobCostByVessel[i].resize(2);
	}

    //apply number of install seasons multiplier to mobilization costs
    for (size_t i = 0; i < mobDemobCostByVessel.size(); i++)
    {
        mobDemobCostByVessel[i][1] = mobDemobCostByVessel[i][1]*number_install_seasons;
    }
	mob_demob_cost = 0;
    for(size_t i = 0; i<mobDemobCostByVessel.size(); i++)
    {
        mob_demob_cost += mobDemobCostByVessel[i][1];
    }

}

//calculate the cost of surveying and verifying cable installation
double wobos_orig::CableRouteSurveyCost()
{
	return (expCabLeng + cab1Leng + cab2Leng)*cabSurveyCR;
}

//calculate the total cost in dollars for the complete assembly and installation of the wind plant
double wobos_orig::TotInstCost()
{
	//create variable to hold total cost values
	double sum = 0;

	//following for loops iterate through cost loops adding each cost to sum
	for (size_t i = 0; i < turbCostsByVessel.size(); i++)
	{
		sum += turbCostsByVessel[i][1];
	}
	for (size_t i = 0; i < subCostsByVessel.size(); i++)
	{
		sum += subCostsByVessel[i][1];
	}
	for (size_t i = 0; i < elecCostsByVessel.size(); i++)
	{
		sum += elecCostsByVessel[i][1];
	}
	for (size_t i = 0; i < mobDemobCostByVessel.size(); i++)
	{
		sum += mobDemobCostByVessel[i][1];
	}

	sum += cabSurvey + cabDrillDist*cabDrillCR + (mpvRentalDR + diveTeamDR + winchDR)*landConstruct + civilWork + elecWork;
	//check substructure type
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		if (substructure == MONOPILE)//check if monopile
		{
			sum += subInstTime*(pileSpreadDR + groutSpreadDR) + groutSpreadMob + pileSpreadMob + compRacks
				+ scourMat*nTurb;
		}
		else
			sum += subInstTime*(pileSpreadDR + groutSpreadDR) + groutSpreadMob + pileSpreadMob + compRacks;
	}
	else
	{
		//check if floating substructure and check anchor type
		if ((substructure == SPAR) || (substructure == SEMISUBMERSIBLE))
		{
			if (anchor == SUCTIONPILE)
			{
				sum += seaSpreadDR*moorTime + seaSpreadMob;
			}
			//check for spar substructure
			if (substructure == SPAR)
			{
				sum += compRacks;
			}
		}
	}
	//return total installation cost
	return sum;
}

//*******************************************************************************************
//Offshore BOS model 'Port & Staging' module starts here and ends after TotalPnSCost()
//function definition
//*******************************************************************************************

//calculate the cost in dollars of port entrance and exit fees based on vessel size and number of entrances/exits
double wobos_orig::EntranceExitCost()
{
	//check substructure type
	if (substructure == SEMISUBMERSIBLE)
	{
		return ((nTurb*turbInstVessel[1] * turbInstVessel[2]) + 1)
			*entranceExitRate;
	}
	else if (substructure == SPAR)
	{
		return (ceil(nTurb / nTurbPerTrip)*(turbInstVessel[1] * turbInstVessel[2] + turbFeederBarge[1]
			* turbFeederBarge[2]) + 1)*entranceExitRate;
	}
	else
	{
		//check installation vessel strategy
		if (installStrategy == PRIMARYVESSEL)
		{
			return ((ceil(nTurb / nTurbPerTrip)*(turbInstVessel[1] * turbInstVessel[2]) +
				ceil(nTurb / nSubPerTrip)*(subInstVessel[1]
				* subInstVessel[2])) + substaInstVessel[1] * substaInstVessel[2])
				*entranceExitRate;
		}
		else
		{
			return (ceil(nTurb / nTurbPerTrip)*(turbFeederBarge[1] * turbFeederBarge[2])
				+ (ceil(nTurb / nSubPerTrip)*subFeederBarge[1] * subFeederBarge[2])
				+ substaInstVessel[1] * substaInstVessel[2])*entranceExitRate;
		}
	}
}

//calculate the cost in dollars of the vessel docking cost as a function of installation duration
double wobos_orig::DockingCost()
{
	//check if floating substructure is selected
	if ((substructure == SEMISUBMERSIBLE) || (substructure == SPAR))
	{
		return (moorTime + floatPrepTime + turbInstTime + subsInstTime)*dockRate;
	}
	else
	{
		return (turbInstTime + subInstTime + subsInstTime)*dockRate;
	}
}

// calculate the cost in dollars of the wharf where loading and unloading operations will take place
double wobos_orig::WharfCost()
{
	if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		return ((rnaM + towerM + subTotM)*nTurb + subsTopM + subsSubM + subsPileM)*wharfRate;
	}
	else
	{
		return ((rnaM + towerM)*nTurb + subsTopM)*wharfRate;
	}
}

//calculate the required area in square meters for substructure staging and pre-assembly
double wobos_orig::SubstructureLaydownArea()
{
	//check installation vessel strategy and if fixed substructure type is selected
	if (installStrategy == FEEDERBARGE)
	{
		return (subDeckArea*nSubPerTrip*subFeederBarge[16]) * 2;
	}
	else if ((substructure == MONOPILE) || (substructure == JACKET))
	{
		return (subDeckArea*nSubPerTrip) * 2;
	}
	else
	{
		return 0;
	}
}

//calculate the cost in dollars of the laydown and staging area for substructures
double wobos_orig::SubstructureLaydownCost()
{
	return subInstTime*laydownCR*subLaydownA;
}

//calculate the required area in square meters for the turbine staging and pre-assembly
double wobos_orig::TurbLaydownArea()
{
	//check installation vessel strategy
	if (installStrategy == FEEDERBARGE)
	{
		return turbDeckArea*nTurbPerTrip*turbFeederBarge[16] * 2;
	}
	else
	{
		return turbDeckArea*nTurbPerTrip * 2;
	}
}

//calculate the cost in dollars of the staging area for the turbines
double wobos_orig::TurbLaydownCost()
{
	return turbInstTime*turbLaydownA*laydownCR;
}


double wobos_orig::NumCranes()
{
	if (nCrane1000 <= 0)
	{
		switch (substructure)
		{
		case SPAR:
			nCrane1000 = 1;
			break;
		case SEMISUBMERSIBLE:
			nCrane1000 = 1;
			break;
		default:
			nCrane1000 = 1;
			break;
		}
	}
	if (nCrane600 <= 0)
	{
		switch (substructure)
		{
		case SPAR:
			nCrane600 = 3;
			break;
		case SEMISUBMERSIBLE:
			nCrane600 = 1;
			break;
		default:
			nCrane600 = 1;
			break;
		}
	}
	return 0;
}
//calculate the cost in dollars of the cranes that are required to carry out lifting operations
//at port
double wobos_orig::CraneCost()
{
	//check substructure type
	switch (substructure)
	{
	case SPAR: case SEMISUBMERSIBLE:
		return (nCrane600*crane600DR + nCrane1000*crane1000DR)*(turbInstTime + floatPrepTime + moorTime)
			+ (crane1000DR*(ceil(subsTopM / 1000))*(placeTop / 24)) + craneMobDemob;
		break;
	default:
		return (nCrane600*crane600DR + nCrane1000*crane1000DR)*(turbInstTime + subInstTime) + craneMobDemob;
	}
}

//calculate the total port cost in dollars which includes the wharf, docking, and entrance/exit costs
double wobos_orig::TotalPortCost()
{
	return entrExitCost + dockCost + wharfCost;
}

//calculate the total staging cost in dollars which includes the laydown, staging, and crane costs
double wobos_orig::TotalStagingCost()
{
	return turbLayCost + subLayCost + craneCost;
}

//calculate the total cost in dollars of all port and staging related costs and fees
double wobos_orig::TotalPnSCost()
{
	return totPortCost + totStageCost;
}

//*******************************************************************************************
//Offshore BOS model 'Engineering & Management' module starts here and ends at TotalEnMCost()
//function definition
//*******************************************************************************************

//calculate the total engineering and management cost based on a percentage of total hard costs
//(hard costs = procurement and installation costs excluding turbine costs)
double wobos_orig::TotalEnMCost()
{
	return estEnMFac*(subTotCost + totPnSCost + totElecCost + totAnICost);
}

//*******************************************************************************************
//Offshore BOS model 'Development' module starts here and ends after TotalDevCost() function
//definition
//*******************************************************************************************

//calculate the cost in dollars of the front end engineering design study pre-development
double wobos_orig::FEEDCost()
{
	return preFEEDStudy + feedStudy;
}

//calculate the total compliance cost in dollars which includes various leases and legal compliance costs
double wobos_orig::PermitsStudiesCompliance()
{
	return stateLease + outConShelfLease + saPlan + conOpPlan + nepaEisMet + physResStudyMet + bioResStudyMet
		+ socEconStudyMet + navStudyMet + nepaEisProj + physResStudyProj + bioResStudyProj + socEconStudyProj
		+ navStudyProj + coastZoneManAct + rivsnHarbsAct + cleanWatAct402 + cleanWatAct404 + faaPlan
		+ endSpecAct + marMamProtAct + migBirdAct + natHisPresAct + addLocPerm;
}

//calculate the cost in dollars of the meteorological tower used for wind resource and other site
//assessment studies including installation costs
double wobos_orig::MetTowerFabnInst()
{
	return nTurb*turbR*metTowCR;
}

//calculate the total development cost in dollars
double wobos_orig::TotalDevCost()
{
        return metFabCost + permStudyComp + feedCost;
}

double wobos_orig::PlantCommissioning()
{
    return (totAnICost + totDevCost + totElecCost + totEnMCost + subTotCost + totPnSCost + turbCapEx*(turbR*nTurb*1000))*plantComm;
}
//*******************************************************************************************
//Offshore BOS model 'Electrical Cable Optimization' module starts here and ends after
//ExportCabCostOptimizer() function definition
//*******************************************************************************************

//This optimizer determines which cables for the array are of the lowest cost considering both
//procurement costs and installation costs
void wobos_orig::ArrayCabCostOptimizer()

{
	size_t nArrVolts = arrayVolt.size();
	size_t nArrCables = arrCables[0].size();

	vector<vector<double> > strings(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > nTurbPS(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > nTurbCab(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > subsInter(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > nTurbInter1(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > nTurbInter2(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > cab1Leng(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > cab2Leng(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > cab1SecM(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > cab2SecM(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > nCab1Sec(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > nCab2Sec(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > instTime(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > cabCost1(nArrVolts, vector<double>(nArrCables));
	vector<vector<double> > cabCost2(nArrVolts, vector<double>(nArrCables));

	//The next 3 arrays contain default data for the array cables. note that if the number of cables
	//changes form the default of 11 then these tables will need the appropriate additions of the
	//data for the cables that you wish to add
	/*                      (turbine      (substation
	($/m)     ,(kg/m)     , $/interface), $/interface)*/
	/*double array33kvData[11][4] =  {
	{185.889 , 20.384   , 8410        , 19610},   //95 mm2
	{202.788 , 22.854   , 8615        , 19815},  //120 mm2
	{208.421 , 23.912   , 8861        , 20062},  //150 mm2
	{236.586 , 25.676   , 9149        , 20349},  //185 mm2
	{270.384 , 28.910   , 9600        , 20800},  //240 mm2
	{315.448 , 32.242   , 10092       , 21292},  //300 mm2
	{360.512 , 37.142   , 10913       , 22113},  //400 mm2
	{422.475 , 42.336   , 11733       , 22933},  //500 mm2
	{478.805 , 48.706   , 12800       , 24000},  //630 mm2
	{585.832 , 57.428   , 14195       , 25395},  //800 mm2
	{698.492 , 66.738   , 15836       , 27036}}; //1000 mm2
	*/
	/*                  (turbine      (substation
	($/m)    ,(kg/m)  , $/interface), $/interface)*/
	/*  double array66kvData[11][4] = {
	{225.320 , 21.6   , 8831        , 20591},  //95 mm2
	{242.219 , 23.8   , 9046        , 20806},  //120 mm2
	{253.485 , 25.7   , 9304        , 21065},  //150 mm2
	{281.650 , 28.0   , 9606        , 21366},  //185 mm2
	{326.714 , 31.3   , 10080       , 21840},  //240 mm2
	{383.044 , 34.3   , 10597       , 22357},  //300 mm2
	{433.741 , 39.2   , 11459       , 23219},  //400 mm2
	{506.970 , 45.4   , 12320       , 24080},  //500 mm2
	{574.566 , 52.0   , 13440       , 25200},  //630 mm2
	{704.125 , 60.1   , 14905       , 26665},  //800 mm2
	{844.950 , 71.7   , 16628       , 28388}}; //1000 mm2

	double currentRating[] =      //amperes
	{300,   //95 mm2
	340,   //120 mm2
	375,   //150 mm2
	420,   //185 mm2
	480,   //240 mm2
	530,   //300 mm2
	590,   //400 mm2
	655,   //500 mm2
	715,   //630 mm2
	775,   //800 mm2
	825};  //1000 mm2*/
	/*
	double Strings(double& cab2CurrRating, double& arrVoltage);
	double NumTurbParStr(double& cab2CurrRating, double& arrVoltage);
	double NumTurbCable1(double& cab1CurrRating, double& arrVoltage);
	double NumTurbCable2(double& cab2CurrRating, double& arrVoltage);
	double InterfacesCable1(double& fullStrings, double& nTurbPS, double& nTurbCab1);
	double InterfacesCable2(double& fullStrings, double& nTurbPS, double&nTurbCab1, double& nTurbCab2);
	double SubstationInterfaces(double& fullStrings, double& nTurbPS);
	double Cable1Length(double& nTurbInter1);
	double Cable2Length(double& nTurbCab1, double& nTurbCab2, double& fullStrings, double& nTurbPS);
	double ArrayCable1Cost(double& cab1Leng, double& cab1CR, double& turbInterCR1, double& nTurbInter1);
	double ArrayCable2Cost(double& cab2Leng, double& cab2CR, double& turbInterCR2, double& nTurbInter2, double& nSubsInter, double& arrSubsInterCR);
	double Cab1SecMass(double& arrCab1Mass);
	double Cab2SecMass(double& arrCab2Mass);
	double Cab1SecPerTrip(double& cab1SecM);
	double Cab2SecPerTrip(double cab2SecM);
	double ArrayCabInstTime(double& cab1Leng, double& cab2Leng, double& nTurbInter1, double& nTurbInter2, double& subsInter,double& cab1SecPerTrip, double& cab2SecPerTrip,
				double& fullStrings, double& nTurbPS, double& nTurbCab1, double nTurbCab2);
	*/
	for (size_t k = 0; k < nArrVolts; k++)
	{
		for (size_t i = 0; i < nArrCables; i++) //this for loop calculates values and stores them in the previously
			//created vectors
		{
			strings[k][i] = Strings(arrCables[k][i][3], arrayVolt[k][0]);

			nTurbPS[k][i] = NumTurbParStr(arrCables[k][i][3], arrayVolt[k][0]);

			nTurbCab[k][i] = NumTurbCable1(arrCables[k][i][3], arrayVolt[k][0]);

			subsInter[k][i] = SubstationInterfaces(strings[k][i], nTurbPS[k][i]);

			cab1SecM[k][i] = Cab1SecMass(arrCables[k][i][2]);

			cab2SecM[k][i] = Cab2SecMass(arrCables[k][i][2]);

			nCab1Sec[k][i] = Cab1SecPerTrip(cab1SecM[k][i]);

			nCab2Sec[k][i] = Cab2SecPerTrip(cab2SecM[k][i]);
		}
	}

	int counter1 = 0;
	int cabIndex1 = 0;
	int cabIndex2 = 0;
	int arrVoltIndex = 0;
	double oldCost= 0;
	double newCost = 0;

	for (size_t k = 0; k < nArrVolts; k++)
	{
		for (size_t i = 0; i < nArrCables; i++)
		{
			for (size_t j = i + 1; j < nArrCables; j++)
			{

			  nTurbInter1[k][i] = InterfacesCable1(strings[k][j], //j should be i
							       nTurbPS[k][j], //j should be i
							       nTurbCab[k][i]);

				nTurbInter2[k][i] = InterfacesCable2(strings[k][j],
					nTurbPS[k][j],
					nTurbCab[k][i],
					nTurbCab[k][j]);

				cab1Leng[k][i] = Cable1Length(nTurbInter1[k][i]);

				cab2Leng[k][i] = Cable2Length(nTurbCab[k][i], nTurbCab[k][j], strings[k][j], nTurbPS[k][j]);

				cabCost1[k][i] = ArrayCable1Cost(cab1Leng[k][i], arrCables[k][i][1], arrCables[k][i][4], nTurbInter1[k][i]);

				cabCost2[k][i] = ArrayCable2Cost(cab2Leng[k][i], arrCables[k][j][1], arrCables[k][j][4], nTurbInter2[k][j], subsInter[k][j], arrCables[k][j][5]);

				instTime[k][i] = ArrayCabInstTime(cab1Leng[k][i], cab2Leng[k][j], //j should be i
								  nTurbInter1[k][i], nTurbInter2[k][j], //j should be i
					nCab1Sec[k][i], nCab2Sec[k][j], strings[k][j],
					nTurbPS[k][j], nTurbCab[k][i], nTurbCab[k][i], nTurbCab[k][j]);

				newCost = cabCost1[k][i] + cabCost2[k][i] + instTime[k][i] * arrCabInstVessel[14]
				  + (cab1Leng[k][i] + cab2Leng[k][j])*cabSurveyCR;//second j should be i?
				if ((k == 0) && (i == 0) && (j == 1))
				{
					oldCost = newCost;
					cabIndex1 = (int)i;
					cabIndex2 = (int)j;
					arrVoltIndex = (int)k;
				}
				else if (newCost < oldCost)
				{
					oldCost = newCost;
					cabIndex1 = (int)i;
					cabIndex2 = (int)j;
					arrVoltIndex = (int)k;
				}

				counter1 += 1;
			}
		}
	}

	arrVoltage = arrayVolt[arrVoltIndex][0];
	cab1CR = arrCables[arrVoltIndex][cabIndex1][1];
	cab2CR = arrCables[arrVoltIndex][cabIndex2][1];
	cab1CurrRating = arrCables[arrVoltIndex][cabIndex1][3];
	cab2CurrRating = arrCables[arrVoltIndex][cabIndex2][3];
	arrCab1Mass = arrCables[arrVoltIndex][cabIndex1][2];
	arrCab2Mass = arrCables[arrVoltIndex][cabIndex2][2];
	cab1TurbInterCR = arrCables[arrVoltIndex][cabIndex1][4];
	cab2TurbInterCR = arrCables[arrVoltIndex][cabIndex2][4];
	cab2SubsInterCR = arrCables[arrVoltIndex][cabIndex2][5];
}


void wobos_orig::ExportCabCostOptimizer()
{
	int nExpVolts = (int)expCabVolt.size();
	int nExpCables = (int)expCables[0].size();

	vector<vector<double> >nExpCab(nExpVolts, vector<double>(nExpCables));
	vector<vector<double> >expCabLeng(nExpVolts, vector<double>(nExpCables));
	vector<vector<double> >expCabCost(nExpVolts, vector<double>(nExpCables));
	vector<vector<double> >expCabSecM(nExpVolts, vector<double>(nExpCables));
	vector<vector<double> >expCabSecPerTrip(nExpVolts, vector<double>(nExpCables));
	vector<vector<double> >expCabInstTime(nExpVolts, vector<double>(nExpCables));

	double newCost = 0;
	double oldCost = 0;
	int expCabIndex = 0;
	int expVoltIndex = 0;

	//($/m) ,(kg/m)  ,(ancillary cost $/interface)
	/*double export132kvData[10][3] =  {
	{433.504  , 48.000 , 57500},  //300 mm2
	{520.489  , 51.100 , 60000},  //400 mm2
	{596.388  , 58.000 , 62500},  //500 mm2
	{689.479  , 65.200 , 65000},  //630 mm2
	{843.823  , 74.000 , 67500},  //800 mm2
	{1006.054 , 85.400 , 70000},  //1000 mm2
	{1168.284 , 113.147, 72500},  //1200 mm2
	{1492.745 , 131.387, 75000},  //1600 mm2
	{1818.332 , 149.627, 77500},  //2000 mm2
	{2223.908 , 172.427, 80000}}; //2500 mm2

	//($/m) ,(kg/m)  ,(ancillary cost $/interface)
	double export220kvData[10][3] =  {
	{495.411  , 71.900 , 57500},  //300 mm2
	{578.187  , 76.500 , 60000},  //400 mm2
	{681.863  , 81.300 , 62500},  //500 mm2
	{788.620  , 86.700 , 65000},  //630 mm2
	{966.623  , 95.300 , 67500},  //800 mm2
	{1159.271 , 104.000, 70000},  //1000 mm2
	{1336.148 , 113.147, 72500},  //1200 mm2
	{1676.499 , 131.387, 75000},  //1600 mm2
	{2042.784 , 149.627, 77500},  //2000 mm2
	{2498.703 , 172.427, 80000}}; //2500 mm2

	double currentRating[] =
	{
	530,   //300 mm2
	590,   //400 mm2
	655,   //500 mm2
	715,   //630 mm2
	775,   //800 mm2
	825,   //1000 mm2
	990,   //1200 mm2
	1061,  //1600 mm2
	1299,  //2000 mm2
	1375}; //2500 mm2*/
	//calculate for each option of voltage 1 the necessary cable data
	for (int k = 0; k < nExpVolts; k++)
	{
		for (int i = 0; i < nExpCables; i++)
		{
			nExpCab[k][i] = NumberExportCable(expCables[k][i][3], expCabVolt[k][0]);

			expCabLeng[k][i] = ExportCableLength(nExpCab[k][i]);

			expCabCost[k][i] = ExportCableCost(expCables[k][i][5], expCables[k][i][1], expCabLeng[k][i], nExpCab[k][i]);

			expCabSecM[k][i] = ExportCableSecMass(expCables[k][i][2], expCabLeng[k][i], nExpCab[k][i]);

			expCabSecPerTrip[k][i] = ExportCabSecPerTrip(expCabSecM[k][i]);

			expCabInstTime[k][i] = ExportCabInstallTime(expCabSecPerTrip[k][i], nExpCab[k][i]);

			newCost = expCabInstTime[k][i] * expCabInstVessel[14] + expCabCost[k][i] + expCabLeng[k][i] * cabSurveyCR;

			if ((k == 0) && (i == 0))
			{
				oldCost = newCost;
				expCabIndex = i;
				expVoltIndex = k;
			}
			else if (newCost < oldCost)
			{
				oldCost = newCost;
				expCabIndex = i;
				expVoltIndex = k;
			}
		}
	}
	expVoltage = expCabVolt[expVoltIndex][0];
	expCurrRating = expCables[expVoltIndex][expCabIndex][3];
	expCabMass = expCables[expVoltIndex][expCabIndex][2];
	expSubsInterCR = expCables[expVoltIndex][expCabIndex][5];
	expCabCR = expCables[expVoltIndex][expCabIndex][1];
}


void wobos_orig::run()
{
  //Initialize new variables with required function return values
  hubD = HubDiameter();
  bladeL = BladeLength();
  chord = HubDiameter();
  nacelleW = NacelleWidth();
  nacelleL = NacelleLength();
  rnaM = RNAMass();
  towerD = TowerDiameter();
  towerM = TowerMass();
  if (mpileL <= 0) //assign monopile length if it is not assigned
    {
      mpileL = MonopileLength();
    }
  if (mpileD <= 0) //assign monopile depth if it is not assigned
    {
      mpileD = turbR;
    }
  mpileM = MonoPileMass();
  mtransM = MonoTransMass();
  mPileCost = MonoPileCost();
  mTransCost = MonoTransCost();
  jlatticeM = JackLatticeMass();
  jtransM = JackTransMass();
  jpileM = JackPileMass();
  jLatticeCost = JackLatticeCost();
  jTransCost = JackTransCost();
  jPileCost = JackPileCost();
  spStifColM = SparStifColMass();
  spTapColM = SparTapColMass();
  spStifColCost = SparStifColCost();
  spTapColCost = SparTapColCost();
  ballM = BallMass();
  ballCost = BallCost();
  ssStifColM = SemiStifColMass();
  ssTrussM = SemiTrussMass();
  ssHeaveM = SemiHeaveMass();
  ssStifColCost = SemiStifColCost();
  ssTrussCost = SemiTrussCost();
  ssHeaveCost = SemiHeaveCost();
  moorSysCost = MooringSys();
  sSteelM = SecondarySteelMass();
  sSteelCost = SecondarySteelCost();
  subTotM = SubstructTotalMass();
  subTotCost = SubstructTotCost();
  systAngle = SystemAngle();
  freeCabLeng = FreeCable();
  fixCabLeng = FixedCable();
  if (cableOptimizer == ON)
    {
      ExportCabCostOptimizer();
    }
  nExpCab = NumberExportCable(expCurrRating, expVoltage);
  nSubstation = NumberSubstation(nExpCab);
  if (cableOptimizer == ON)
    {
      ArrayCabCostOptimizer();
    }
  fullStrings = Strings(cab2CurrRating, arrVoltage);
  nTurbPS = NumTurbParStr(cab2CurrRating, arrVoltage);
  nTurbCab1 = NumTurbCable1(cab1CurrRating, arrVoltage);
  nTurbCab2 = NumTurbCable2(cab2CurrRating, arrVoltage);
  nTurbInter1 = InterfacesCable1(fullStrings, nTurbPS, nTurbCab1);
  nTurbInter2 = InterfacesCable2(fullStrings, nTurbPS, nTurbCab1, nTurbCab2);
  nSubsInter = SubstationInterfaces(fullStrings, nTurbPS);
  cab1Leng = Cable1Length(nTurbInter1);
  cab2Leng = Cable2Length(nTurbCab1, nTurbCab2, fullStrings, nTurbPS);
  expCabLeng = ExportCableLength(nExpCab);
  nMPT = NumberMPT();
  mptRating = SingleMPTRating();
  mptCost = MPTCost();
  subsTopM = SubstationTopsideMass();
  subsTopCost = SubstationTopsideCost();
  arrCab1Cost = ArrayCable1Cost(cab1Leng, cab1CR, cab1TurbInterCR, nTurbInter1);
  arrCab2Cost = ArrayCable2Cost(cab2Leng, cab2CR, cab2TurbInterCR, nTurbInter2, nSubsInter, cab2SubsInterCR);
  expCabCost = ExportCableCost(expSubsInterCR, expCabCR, expCabLeng, nExpCab);
  shuntReactors = ShuntReactors();
  switchGear = Switchgear();
  ancillarySys = AncillarySystems();
  subsSubM = SubstationSubMass();
  subsPileM = SubstationSubPileMass();
  subsLandAssembly = LandTopsideAssembly();
  subsSubCost = SubstationSubCost();
  switchYard = SwitchYardCost();
  onShoreSubs = OnshoreSubsCost();
  onshoreMisc = OnshoreSubsMiscCost();
  transLine = TransLineCost();
  subCabCost = SubseaCableCost();
  offSubsCost = OffshoreSubstationCost();
  onshoreTransCost = OnshoreTransCost();
  totElecCost = TotElectricalCost();
  moorTime = MooringSysInstall();
  floatPrepTime = PrepFloatSubstructure();
  turbDeckArea = MinTurbDeckArea();
  nTurbPerTrip = TurbsPerTrip();
  turbInstTime = TurbineInstall();
  subDeckArea = MinSubDeckArea();
  nSubPerTrip = SubPerTrip();
  subInstTime = SubstructureInstTime();
  cab1SecM = Cab1SecMass(arrCab1Mass);
  cab2SecM = Cab2SecMass(arrCab2Mass);
  cab1SecPerTrip = Cab1SecPerTrip(cab1SecM);
  cab2SecPerTrip = Cab2SecPerTrip(cab2SecM);
  arrInstTime = ArrayCabInstTime(cab1Leng, cab2Leng, nTurbInter1, nTurbInter2, nSubsInter,
				 cab1SecPerTrip, cab2SecPerTrip, fullStrings, nTurbPS, nTurbCab1, nTurbCab2);
  expCabSecM = ExportCableSecMass(expCabMass, expCabLeng, nExpCab);
  expCabSecPerTrip = ExportCabSecPerTrip(expCabSecM);
  expInstTime = ExportCabInstallTime(expCabSecPerTrip, nExpCab);
  subsInstTime = SubsInstallTime();
  totInstTime = TotalInstallTime();
  cabSurvey = CableRouteSurveyCost();
  TurbInstCost();
  SubInstCost();
  ElectricalInstCost();
  VesselMobDemobCost();
  totAnICost = TotInstCost();
  entrExitCost = EntranceExitCost();
  wharfCost = WharfCost();
  dockCost = DockingCost();
  subLaydownA = SubstructureLaydownArea();
  subLayCost = SubstructureLaydownCost();
  turbLaydownA = TurbLaydownArea();
  turbLayCost = TurbLaydownCost();
  NumCranes();
  craneCost = CraneCost();
  totPortCost = TotalPortCost();
  totStageCost = TotalStagingCost();
  totPnSCost = TotalPnSCost();
  totEnMCost = TotalEnMCost();
  feedCost = FEEDCost();
  permStudyComp = PermitsStudiesCompliance();
  metFabCost = MetTowerFabnInst();
  decomCost = DecomissExpense();
  totDevCost = TotalDevCost();
  commissioning = PlantCommissioning();
  Construction_finance_factor();
  Soft_costs();
  Total_bos_cost();
}
