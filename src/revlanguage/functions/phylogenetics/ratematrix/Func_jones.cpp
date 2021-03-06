#include <iosfwd>

#include "Func_jones.h"
#include "RateMatrix_Jones.h"
#include "RlRateMatrix.h"
#include "ArgumentRules.h"
#include "Procedure.h"
#include "RevPtr.h"
#include "RevVariable.h"
#include "RlFunction.h"
#include "RlRateGenerator.h"
#include "TypeSpec.h"

using namespace RevLanguage;

/** default constructor */
Func_jones::Func_jones( void ) : Procedure( )
{
    
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
Func_jones* Func_jones::clone( void ) const
{
    
    return new Func_jones( *this );
}


RevPtr<RevVariable> Func_jones::execute()
{
    

    RevBayesCore::RateMatrix_Jones *rmj = new RevBayesCore::RateMatrix_Jones();
    RateMatrix* value = new RateMatrix( rmj );
    
    return new RevVariable( value );
}


/* Get argument rules */
const ArgumentRules& Func_jones::getArgumentRules( void ) const
{
    
    static ArgumentRules argumentRules = ArgumentRules();
    
    return argumentRules;
}


const std::string& Func_jones::getClassType(void)
{
    
    static std::string rev_type = "Func_jones";
    
	return rev_type; 
}

/* Get class type spec describing type of object */
const TypeSpec& Func_jones::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
	return rev_type_spec; 
}


/**
 * Get the primary Rev name for this function.
 */
std::string Func_jones::getFunctionName( void ) const
{
    // create a name variable that is the same for all instance of this class
    std::string f_name = "fnJones";
    
    return f_name;
}


/* Get return type */
const TypeSpec& Func_jones::getReturnType( void ) const
{
    
    static TypeSpec return_typeSpec = RateGenerator::getClassTypeSpec();
    
    return return_typeSpec;
}


const TypeSpec& Func_jones::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}
