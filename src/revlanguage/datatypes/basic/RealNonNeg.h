/**
 * @file
 * This file contains the declaration of RealNonNeg, which
 * is the primitive RevBayes type for positive real numbers.
 *
 * @brief Declaration of RealNonNeg
 *
 * (c) Copyright 2009-
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 *
 * $Id$
 */

#ifndef RealNonNeg_H
#define RealNonNeg_H

#include "Real.h"

#include <ostream>
#include <string>

namespace RevLanguage {

    class Natural;
    
    class RealNonNeg : public Real {

        public:
        RealNonNeg(void);                                                           //!< Default constructor
        RealNonNeg(RevBayesCore::TypedDagNode<double> *x);                          //!< Construct from double
        RealNonNeg(double x);                                                       //!< Construct from double
        RealNonNeg(long x);                                                         //!< Construct from int

        // Basic operator functions
        virtual RevObject*              add(const RevObject &rhs) const;            //!< Addition operator used for example in '+=' statements
        RealNonNeg*                     add(const Natural &rhs) const;              //!< Addition operator used for example in '+=' statements
        RealNonNeg*                     add(const RealNonNeg &rhs) const;           //!< Addition operator used for example in '+=' statements
        virtual RevObject*              divide(const RevObject &rhs) const;         //!< Division operator used for example in '/=' statements
        RealNonNeg*                     divide(const Natural &rhs) const;           //!< Division operator used for example in '/=' statements
        RealNonNeg*                     divide(const RealNonNeg &rhs) const;        //!< Division operator used for example in '/=' statements
        virtual RevObject*              multiply(const RevObject &rhs) const;       //!< Multiplication operator used for example in '*=' statements
        RealNonNeg*                     multiply(const Natural &rhs) const;         //!< Multiplication operator used for example in '*=' statements
        RealNonNeg*                     multiply(const RealNonNeg &rhs) const;      //!< Multiplication operator used for example in '*=' statements

        // Basic utility functions
        virtual RealNonNeg*             clone(void) const;                          //!< Clone object
        virtual RevObject*              convertTo(const TypeSpec& type) const;      //!< Convert to type
        static const std::string&       getClassType(void);                         //!< Get Rev type
        static const TypeSpec&          getClassTypeSpec(void);                     //!< Get class type spec
        virtual const TypeSpec&         getTypeSpec(void) const;                    //!< Get language type of the object
        virtual double                  isConvertibleTo(const TypeSpec& type, bool once) const;
    
        std::string                     getGuiName(void) { return "Non Negative Real"; }
        std::string                     getGuiUnicodeSymbol(void) { return "+R0"; }
        std::string                     getGuiInfo(void) { return ""; }
    };
    
}

#endif

