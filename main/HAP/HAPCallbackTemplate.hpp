//
// HAPCallbackTemplate.hpp
// Homekit
//
//  Created on: 24.06.2017
//      Author: michael
//
//  https://stackoverflow.com/questions/1000663/using-a-c-class-member-function-as-a-c-callback-function
// 

#ifndef HAPCALLBACKTEMPLATE_HPP_
#define HAPCALLBACKTEMPLATE_HPP_

#include <functional>

template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
   template <typename... Args> 
   static Ret callback(Args... args) {                    
      return func(args...);  
   }
   static std::function<Ret(Params...)> func; 
};

// Initialize the static member.
template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;

#endif