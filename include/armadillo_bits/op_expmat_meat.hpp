// Copyright (C) 2014-2016 National ICT Australia (NICTA)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// -------------------------------------------------------------------
// 
// Written by Conrad Sanderson - http://conradsanderson.id.au



//! \addtogroup op_expmat
//! @{


//! implementation based on:
//! Cleve Moler, Charles Van Loan.
//! Nineteen Dubious Ways to Compute the Exponential of a Matrix, Twenty-Five Years Later.
//! SIAM Review, Vol. 45, No. 1, 2003, pp. 3-49.
//! http://dx.doi.org/10.1137/S00361445024180


template<typename T1>
inline
void
op_expmat::apply(Mat<typename T1::elem_type>& out, const Op<T1, op_expmat>& expr)
  {
  arma_extra_debug_sigprint();
  
  const bool status = op_expmat::apply_direct(out, expr.m);
  
  if(status == false)
    {
    out.reset();
    arma_stop_runtime_error("expmat(): given matrix appears ill-conditioned");
    }
  }



template<typename T1>
inline
bool
op_expmat::apply_direct(Mat<typename T1::elem_type>& out, const Base<typename T1::elem_type, T1>& expr)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  typedef typename T1::pod_type   T;
  
  if(is_op_diagmat<T1>::value)
    {
    out = expr.get_ref();  // force the evaluation of diagmat()
    
    arma_debug_check( (out.is_square() == false), "expmat(): given matrix must be square sized" );
    
    const uword N = (std::min)(out.n_rows, out.n_cols);
    
    for(uword i=0; i<N; ++i)
      {
      out.at(i,i) = std::exp( out.at(i,i) );
      }
    }
  else
    {
    Mat<eT> A = expr.get_ref();
    
    arma_debug_check( (A.is_square() == false), "expmat(): given matrix must be square sized" );
    
    const T norm_val = arma::norm(A, "inf");
    
    const double log2_val = (norm_val > T(0)) ? double(eop_aux::log2(norm_val)) : double(0);
    
    int exponent = int(0);  std::frexp(log2_val, &exponent);
    
    const uword s = uword( (std::max)(int(0), exponent + int(1)) );
    
    A /= eT(eop_aux::pow(double(2), double(s)));
    
    T c = T(0.5);
    
    Mat<eT> E(A.n_rows, A.n_rows, fill::eye);  E += c * A;
    Mat<eT> D(A.n_rows, A.n_rows, fill::eye);  D -= c * A;
    
    Mat<eT> X = A;
    
    bool positive = true;
    
    const uword N = 6;
    
    for(uword i = 2; i <= N; ++i)
      {
      c = c * T(N - i + 1) / T(i * (2*N - i + 1));
      
      X = A * X;
      
      E += c * X;
      
      if(positive)  { D += c * X; }  else  { D -= c * X; }
      
      positive = (positive) ? false : true;
      }
    
    if( (D.is_finite() == false) || (E.is_finite() == false) )  { return false; }
    
    const bool status = solve(out, D, E);
    
    if(status == false)  { return false; }
    
    for(uword i=0; i < s; ++i)  { out = out * out; }
    }
  
  return true;
  }



//! @}
