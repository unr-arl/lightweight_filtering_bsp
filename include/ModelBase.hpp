/*
 * ModelBase.hpp
 *
 *  Created on: Feb 9, 2014
 *      Author: Bloeschm
 */

#ifndef ModelBase_HPP_
#define ModelBase_HPP_

#include <Eigen/Dense>
#include <iostream>
#include "kindr/rotations/RotationEigen.hpp"

namespace LWF{

template<typename Input, typename Output, typename Meas, typename Noise>
class ModelBase{
 public:
  typedef Input mtInput;
  typedef Output mtOutput;
  typedef Meas mtMeas;
  typedef Noise mtNoise;
  typedef Eigen::Matrix<double,mtOutput::D_,mtInput::D_> mtJacInput;
  typedef Eigen::Matrix<double,mtOutput::D_,mtNoise::D_> mtJacNoise;
  typedef Eigen::Matrix<double,mtNoise::D_,1> mtNoiseVector;
  ModelBase(){};
  virtual ~ModelBase(){};
  virtual mtOutput eval(const mtInput& input, const mtMeas& meas, double dt = 0.0) const{
    mtNoise noise;
    noise.setIdentity();
    return eval(input,meas,noise,dt);
  }
  virtual mtOutput eval(const mtInput& input, const mtMeas& meas, const mtNoise noise, double dt = 0.0) const{
    mtOutput output;
    return output;
  }
  virtual mtJacInput jacInput(const mtInput& input, const mtMeas& meas, double dt = 0.0) const{
    mtJacInput J;
    J.setZero();
    return J;
  }
  virtual mtJacNoise jacNoise(const mtInput& input, const mtMeas& meas, double dt = 0.0) const{
    mtJacNoise J;
    J.setZero();
    return J;
  }
  mtJacInput jacInputFD(const mtInput& input, const mtMeas& meas, double dt, double d){
    mtJacInput F;
    mtInput inputDisturbed;
    mtOutput outputReference = eval(input,meas,dt);
    typename mtInput::mtCovMat I;
    typename mtOutput::mtDifVec dif;
    I.setIdentity();
    for(unsigned int i=0;i<mtInput::D_;i++){
      input.boxPlus(d*I.col(i),inputDisturbed);
      eval(inputDisturbed,meas,dt).boxMinus(outputReference,dif);
      F.col(i) = dif/d;
    }
    return F;
  }
  mtJacNoise jacNoiseFD(const mtInput& input, const mtMeas& meas, double dt, double d){
    mtNoise noise;
    noise.setIdentity();
    mtJacNoise H;
    mtNoise noiseDisturbed;
    mtOutput outputReference = eval(input,meas,noise,dt);
    typename mtNoise::mtCovMat I;
    typename mtOutput::mtDifVec dif;
    I.setIdentity();
    for(unsigned int i=0;i<mtNoise::D_;i++){
      noise.boxPlus(d*I.col(i),noiseDisturbed);
      eval(input,meas,noiseDisturbed,dt).boxMinus(outputReference,dif);
      H.col(i) = dif/d;
    }
    return H;
  }
  void testJacInput(double d = 1e-6,double th = 1e-6,double s = 0,double dt = 0.1){
    mtInput input;
    mtMeas meas;
    input.setRandom(s);
    meas.setRandom(s);
    double r = (jacInput(input,meas,dt)-jacInputFD(input,meas,dt,d)).maxCoeff();
    if(r>th){
      std::cout << "==== Model jacInput Test failed: " << r << " is larger than " << th << " ====" << std::endl;
    } else {
      std::cout << "==== Test successful (" << r << ") ====" << std::endl;
    }
  }
  void testJacNoise(double d = 1e-6,double th = 1e-6,double s = 0,double dt = 0.1){
    mtInput input;
    mtMeas meas;
    input.setRandom(s);
    meas.setRandom(s);
    double r = (jacNoise(input,meas,dt)-jacNoiseFD(input,meas,dt,d)).maxCoeff();
    if(r>th){
      std::cout << "==== Model jacNoise Test failed: " << r << " is larger than " << th << " ====" << std::endl;
    } else {
      std::cout << "==== Test successful (" << r << ") ====" << std::endl;
    }
  }
  void testJacs(double d = 1e-6,double th = 1e-6,double s = 0,double dt = 0.1){
    testJacInput(d,th,s,dt);
    testJacNoise(d,th,s,dt);
  }
};

}

#endif /* ModelBase_HPP_ */
