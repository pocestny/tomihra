#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__
#include <functional>
#include <unordered_map>
#include <string>
#include <tuple>
#include <vector>
#include <utility>
#include "hash.h"

/*
 * a simplistic implementation of signal/slot mechanism like
 *
 * Conenctor<int>::connect(&object1, "hello", [](int x) { ... process x...});
 * Connector<int>::emit(this,"hello",4);
 *
 *
 */


// hmm... doesn't work with void args.... nevermind
template <typename... Args>
class Connector {
  public:
    // sender: address of sending object, name of signal
    using Sender = std::pair<void *,std::string>;
    using Slot = std::function<void(Args...)>;
    using Map = std::unordered_map<Sender,std::vector<std::pair<Slot,int>>>;

    // return ID for possible deletion
    template <typename T>
    static int connect(T* sender, std::string signal, Slot recv);
    template <typename T>
    static void emit(T* sender,std::string signal,Args... args);
   
    // todo disconnect etc
    // todo multiplexed listening (to signal from any sender)

  private:
    static Map _map;
    static int _id;
};

template <typename... Args>
template <typename T>
int Connector<Args...>::connect(T* sender, std::string signal,
                                Connector<Args...>::Slot recv) {
  _map[make_pair(static_cast<void *>(sender),signal)].push_back(make_pair(recv, _id));
  return _id++;
}

template <typename... Args>
template <typename T>
void Connector<Args...>::emit(T* sender, std::string signal,
                                     Args... args) {
  auto x = make_pair(static_cast<void*>(sender),signal);
  if (_map.count(x)==0) return;
  for (auto recv : _map[x])
    recv.first(args...);
}


template <typename... Args>
typename Connector<Args...>::Map Connector<Args...>::_map =
    Connector<Args...>::Map();

template <typename... Args>
int Connector<Args...>::_id = 0;

#endif
