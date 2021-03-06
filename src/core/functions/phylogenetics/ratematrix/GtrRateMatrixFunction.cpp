#include "GtrRateMatrixFunction.h"

#include <vector>

#include "Cloneable.h"
#include "RateMatrix_GTR.h"
#include "Simplex.h"
#include "TypedDagNode.h"

namespace RevBayesCore { class DagNode; }

using namespace RevBayesCore;
/**
 * Default constructor.
 *
 * This function takes two inputs:
 * @param er The simplex of exchangeabilities
 * @param bf The simplex of stationary base frequencies
 */

GtrRateMatrixFunction::GtrRateMatrixFunction(const TypedDagNode< Simplex > *er, const TypedDagNode< Simplex > *bf) : TypedFunction<RateGenerator>( new RateMatrix_GTR(bf->getValue().size()) ),
    exchangeability_rates( er ),
    base_frequencies( bf )
{
    // add the lambda parameter as a parent
    addParameter( base_frequencies );
    addParameter( exchangeability_rates );
    
    update();
}

GtrRateMatrixFunction::~GtrRateMatrixFunction( void )
{
    // We don't delete the parameters, because they might be used somewhere else too. The model needs to do that!
}



GtrRateMatrixFunction* GtrRateMatrixFunction::clone( void ) const
{
    return new GtrRateMatrixFunction( *this );
}


void GtrRateMatrixFunction::update( void )
{

    // get the information from the arguments for reading the file
    const std::vector<double>& r = exchangeability_rates->getValue();
    const std::vector<double>& f = base_frequencies->getValue();
    
    
    // set the base frequencies
    static_cast< RateMatrix_GTR* >(value)->setStationaryFrequencies( f );
    static_cast< RateMatrix_GTR* >(value)->setExchangeabilityRates( r );
    
    value->update();
    
}



void GtrRateMatrixFunction::swapParameterInternal(const DagNode *oldP, const DagNode *newP)
{

    if (oldP == base_frequencies)
    {
        base_frequencies = static_cast<const TypedDagNode< Simplex >* >( newP );
    }
    else if (oldP == exchangeability_rates)
    {
        exchangeability_rates = static_cast<const TypedDagNode< Simplex >* >( newP );
    }

}


