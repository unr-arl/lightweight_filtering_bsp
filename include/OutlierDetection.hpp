/*
 * OutlierDetection.hpp
 *
 *  Created on: Feb 9, 2014
 *      Author: Bloeschm
 */

#ifndef OUTLIERDETECTION_HPP_
#define OUTLIERDETECTION_HPP_

#include <Eigen/Dense>
#include "PropertyHandler.hpp"

namespace LWF{

template<unsigned int S, unsigned int D, unsigned int N = 1> struct ODEntry{
  static const unsigned int S_ = S;
  static const unsigned int D_ = D;
};

template<unsigned int S, unsigned int D>
class OutlierDetectionBase{
 public:
  static const unsigned int S_ = S;
  static const unsigned int D_ = D;
  bool outlier_;
  bool enabled_;
  double mahalanobisTh_;
  unsigned int outlierCount_;
  OutlierDetectionBase(){
    mahalanobisTh_ = -0.0376136*D_*D_+1.99223*D_+2.05183; // Quadratic fit to chi square
    enabled_ = false;
    outlier_ = false;
    outlierCount_ = 0;
  }
  virtual ~OutlierDetectionBase(){};
  template<int E>
  void check(const Eigen::Matrix<double,E,1>& innVector,const Eigen::Matrix<double,E,E>& Py){
    const double d = ((innVector.block(S_,0,D_,1)).transpose()*Py.block(S_,S_,D_,D_).inverse()*innVector.block(S_,0,D_,1))(0,0);
    outlier_ = d > mahalanobisTh_;
    if(outlier_){
      outlierCount_++;
    } else {
      outlierCount_ = 0;
    }
  }
  virtual void registerToPropertyHandler(PropertyHandler* mpPropertyHandler, const std::string& str, unsigned int i = 0) = 0;
  virtual void reset() = 0;
  virtual bool isOutlier(unsigned int i) const = 0;
  virtual void setEnabled(unsigned int i,bool enabled) = 0;
  virtual void setEnabledAll(bool enabled) = 0;
  virtual unsigned int& getCount(unsigned int i) = 0;
  virtual double& getMahalTh(unsigned int i) = 0;
};

template<unsigned int S, unsigned int D,typename T>
class OutlierDetectionConcat: public OutlierDetectionBase<S,D>{
 public:
  using OutlierDetectionBase<S,D>::S_;
  using OutlierDetectionBase<S,D>::D_;
  using OutlierDetectionBase<S,D>::outlier_;
  using OutlierDetectionBase<S,D>::enabled_;
  using OutlierDetectionBase<S,D>::mahalanobisTh_;
  using OutlierDetectionBase<S,D>::outlierCount_;
  using OutlierDetectionBase<S,D>::check;
  T sub_;
  template<int dI, int dS>
  void doOutlierDetection(const Eigen::Matrix<double,dI,1>& innVector,Eigen::Matrix<double,dI,dI>& Py,Eigen::Matrix<double,dI,dS>& H){
    static_assert(dI>=S+D,"Outlier detection out of range");
    check(innVector,Py);
    sub_.doOutlierDetection(innVector,Py,H);
    if(outlier_ && enabled_){
      Py.block(0,S_,dI,D_).setZero();
      Py.block(S_,0,D_,dI).setZero();
      Py.block(S_,S_,D_,D_).setIdentity();
      H.block(S_,0,D_,dS).setZero();
    }
  }
  void registerToPropertyHandler(PropertyHandler* mpPropertyHandler, const std::string& str, unsigned int i = 0){
    mpPropertyHandler->doubleRegister_.registerScalar(str + std::to_string(i), mahalanobisTh_);
    sub_.registerToPropertyHandler(mpPropertyHandler,str,i+1);
  }
  void reset(){
    outlier_ = false;
    outlierCount_ = 0;
    sub_.reset();
  }
  bool isOutlier(unsigned int i) const{
    if(i==0){
      return outlier_;
    } else {
      return sub_.isOutlier(i-1);
    }
  }
  void setEnabled(unsigned int i,bool enabled){
    if(i==0){
      enabled_ = enabled;
    } else {
      sub_.setEnabled(i-1,enabled);
    }
  }
  void setEnabledAll(bool enabled){
    enabled_ = enabled;
    sub_.setEnabledAll(enabled);
  }
  unsigned int& getCount(unsigned int i){
    if(i==0){
      return outlierCount_;
    } else {
      return sub_.getCount(i-1);
    }
  }
  double& getMahalTh(unsigned int i){
    if(i==0){
      return mahalanobisTh_;
    } else {
      return sub_.getMahalTh(i-1);
    }
  }
};

class OutlierDetectionDefault: public OutlierDetectionBase<0,0>{
 public:
  using OutlierDetectionBase<0,0>::mahalanobisTh_;
  using OutlierDetectionBase<0,0>::outlierCount_;
  template<int dI, int dS>
  void doOutlierDetection(const Eigen::Matrix<double,dI,1>& innVector,Eigen::Matrix<double,dI,dI>& Py,Eigen::Matrix<double,dI,dS>& H){
  }
  void registerToPropertyHandler(PropertyHandler* mpPropertyHandler, const std::string& str, unsigned int i = 0){
  }
  void reset(){
  }
  bool isOutlier(unsigned int i) const{
    assert(0);
    return false;
  }
  void setEnabled(unsigned int i,bool enabled){
    assert(0);
  }
  void setEnabledAll(bool enabled){
  }
  unsigned int& getCount(unsigned int i){
    assert(0);
    return outlierCount_;
  }
  double& getMahalTh(unsigned int i){
    assert(0);
    return mahalanobisTh_;
  }
};

template<typename... ODEntries>
class OutlierDetection{};
template<unsigned int S, unsigned int D, unsigned int N, typename... ODEntries>
class OutlierDetection<ODEntry<S,D,N>,ODEntries...>: public OutlierDetectionConcat<S,D,OutlierDetection<ODEntry<S+D,D,N-1>,ODEntries...>>{};
template<unsigned int S, unsigned int D, typename... ODEntries>
class OutlierDetection<ODEntry<S,D,1>,ODEntries...>: public OutlierDetectionConcat<S,D,OutlierDetection<ODEntries...>>{};
template<unsigned int S, unsigned int D, typename... ODEntries>
class OutlierDetection<ODEntry<S,D,0>,ODEntries...>: public OutlierDetection<ODEntries...>{};
template<unsigned int S, unsigned int D, unsigned int N>
class OutlierDetection<ODEntry<S,D,N>>: public OutlierDetectionConcat<S,D,OutlierDetection<ODEntry<S+D,D,N-1>>>{};
template<unsigned int S, unsigned int D>
class OutlierDetection<ODEntry<S,D,1>>: public OutlierDetectionConcat<S,D,OutlierDetectionDefault>{};
template<unsigned int S, unsigned int D>
class OutlierDetection<ODEntry<S,D,0>>: public OutlierDetectionDefault{};

}

#endif /* OUTLIERDETECTION_HPP_ */