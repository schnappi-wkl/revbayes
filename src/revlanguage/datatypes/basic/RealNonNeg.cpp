#include <sstream>
#include <string>

#include "Natural.h"
#include "Probability.h"
#include "RealNonNeg.h"
#include "RbException.h"
#include "TypeSpec.h"
#include "Real.h"
#include "RevObject.h"
#include "TypedDagNode.h"

using namespace RevLanguage;
    
/** Default constructor */
RealNonNeg::RealNonNeg( void ) : Real( 1.0 )
{

}


/** Construct from double */
RealNonNeg::RealNonNeg( RevBayesCore::TypedDagNode<double> *x ) : Real( x )
{
    
    if ( x->getValue() < 0.0 )
    {
        throw RbException( "Negative value for " + getClassType() );
    }
    
}


/** Construct from double */
RealNonNeg::RealNonNeg( double x ) : Real( x )
{

    if ( x < 0.0 )
    {
        throw RbException( "Negative value for " + getClassType() );
    }
    
}


/** Construct from int */
RealNonNeg::RealNonNeg( long x ) : Real( double(x) )
{

    if ( x < 0 )
    {
        throw RbException( "Negative value for " + getClassType() );
    }
    
}


/**
 * Generic addition operator.
 * We test if the rhs is of a type that we use for a specialized addition operation.
 *
 * \param[in]   rhs     The right hand side operand of the addition operation.
 *
 * \return              A new object holding the sum.
 */
RevObject* RealNonNeg::add( const RevObject& rhs ) const
{
    
    if ( rhs.getTypeSpec().isDerivedOf( RealNonNeg::getClassTypeSpec() ) )
    {
        return add( static_cast<const RealNonNeg&>( rhs ) );
    }
    
    if ( rhs.getTypeSpec().isDerivedOf( Natural::getClassTypeSpec() ) )
    {
        return add( static_cast<const Natural&>( rhs ) );
    }
    
    return Real::add( rhs );
}


/**
 * Specialized addition operation between two positive real numbers.
 * The return value is also of type positive real number.
 *
 * \param[in]   rhs     The right hand side operand of the addition operation.
 *
 * \return              A new object holding the sum.
 */
RealNonNeg* RealNonNeg::add(const RevLanguage::Natural &rhs) const
{
    
    RealNonNeg *n = new RealNonNeg( dag_node->getValue() + rhs.getValue() );
    
    return n;
}


/**
 * Specialized addition operation between two positive real numbers.
 * The return value is also of type positive real number.
 *
 * \param[in]   rhs     The right hand side operand of the addition operation.
 *
 * \return              A new object holding the sum.
 */
RealNonNeg* RealNonNeg::add(const RevLanguage::RealNonNeg &rhs) const
{
    
    RealNonNeg *n = new RealNonNeg( dag_node->getValue() + rhs.getValue() );
    
    return n;
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
RealNonNeg* RealNonNeg::clone( void ) const
{

	return new RealNonNeg( *this );
}

RevObject* RealNonNeg::convertTo( const TypeSpec& type ) const
{
    
    if ( type == Real::getClassTypeSpec() )
    {
        return new Real(dag_node->getValue());
    }
    
    return Real::convertTo( type );
}


/**
 * Generic division operator.
 * We test if the rhs is of a type that we use for a specialized division operation.
 *
 * \param[in]   rhs     The right hand side operand of the division operation.
 *
 * \return              A new object holding the ratio.
 */
RevObject* RealNonNeg::divide( const RevObject& rhs ) const
{
    
    if ( rhs.getTypeSpec().isDerivedOf( RealNonNeg::getClassTypeSpec() ) )
    {
        return divide( static_cast<const RealNonNeg&>( rhs ) );
    }
    
    if ( rhs.getTypeSpec().isDerivedOf( Natural::getClassTypeSpec() ) )
    {
        return divide( static_cast<const Natural&>( rhs ) );
    }
    
    return Real::divide( rhs );
}


/**
 * Specialized division operation between two positive real numbers.
 * The return value is also of type positive real number.
 *
 * \param[in]   rhs     The right hand side operand of the division operation.
 *
 * \return              A new object holding the ratio.
 */
RealNonNeg* RealNonNeg::divide(const RevLanguage::Natural &rhs) const
{
    
    RealNonNeg *n = new RealNonNeg( dag_node->getValue() / rhs.getValue() );
    
    return n;
}


/**
 * Specialized division operation between two positive real numbers.
 * The return value is also of type positive real number.
 *
 * \param[in]   rhs     The right hand side operand of the division operation.
 *
 * \return              A new object holding the ratio.
 */
RealNonNeg* RealNonNeg::divide(const RevLanguage::RealNonNeg &rhs) const
{
    
    RealNonNeg *n = new RealNonNeg( dag_node->getValue() / rhs.getValue() );
    
    return n;
}


/** Get Rev type of object */
const std::string& RealNonNeg::getClassType(void)
{
    
    static std::string rev_type = "RealNonNeg";
    
	return rev_type; 
}

/** Get class type spec describing type of object */
const TypeSpec& RealNonNeg::getClassTypeSpec(void) {
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( Real::getClassTypeSpec() ) );
    
	return rev_type_spec; 
}


/** Get type spec */
const TypeSpec& RealNonNeg::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}


/** Is convertible to type? */
double RealNonNeg::isConvertibleTo(const TypeSpec& type, bool once) const
{
    
    if ( type == Real::getClassTypeSpec() )
    {
        return 0.2;
    }
    
    double tmp = Real::isConvertibleTo(type, once);
    return ( (tmp == -1.0) ? -1.0 : (tmp+0.2));
}


/**
 * Generic multiplication operator.
 * We test if the rhs is of a type that we use for a specialized multiplication operation.
 *
 * \param[in]   rhs     The right hand side operand of the multiplication operation.
 *
 * \return              A new object holding the product.
 */
RevObject* RealNonNeg::multiply( const RevObject& rhs ) const
{
    
    if ( rhs.getTypeSpec().isDerivedOf( RealNonNeg::getClassTypeSpec() ) )
    {
        return multiply( static_cast<const RealNonNeg&>( rhs ) );
    }
    
    if ( rhs.getTypeSpec().isDerivedOf( Natural::getClassTypeSpec() ) )
    {
        return multiply( static_cast<const Natural&>( rhs ) );
    }
    
    return Real::multiply( rhs );
}


/**
 * Specialized multiplication operation between two positive real numbers.
 * The return value is also of type positive real number.
 *
 * \param[in]   rhs     The right hand side operand of the multiplication operation.
 *
 * \return              A new object holding the product.
 */
RealNonNeg* RealNonNeg::multiply(const RevLanguage::Natural &rhs) const
{
    
    RealNonNeg *n = new RealNonNeg( dag_node->getValue() * rhs.getValue() );
    
    return n;
}


/**
 * Specialized multiplication operation between two positive real numbers.
 * The return value is also of type positive real number.
 *
 * \param[in]   rhs     The right hand side operand of the multiplication operation.
 *
 * \return              A new object holding the product.
 */
RealNonNeg* RealNonNeg::multiply(const RevLanguage::RealNonNeg &rhs) const
{
    
    RealNonNeg *n = new RealNonNeg( dag_node->getValue() * rhs.getValue() );
    
    return n;
}
