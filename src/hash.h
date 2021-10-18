#ifndef __HASH_H__
#define __HASH_H__

/*
 * hash functions for vectors, keys, etc
 */

namespace std{
// https://www.variadic.xyz/2018/01/15/hashing-stdpair-and-stdtuple/

template <typename T>
inline void hash_combine(size_t& seed, const T& val) {
  hash<T> hasher;
  seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

//  taken from https://stackoverflow.com/a/7222201/916549
//
// HASH PAIR
template <typename S, typename T>
struct hash<pair<S, T>> {
  inline size_t operator()(const pair<S, T>& val) const {
    size_t seed = 0;
    hash_combine(seed, val.first);
    hash_combine(seed, val.second);
    return seed;
  }
};

// HASH TUPLE

template <class... TupleArgs>
struct hash<tuple<TupleArgs...>> {
 private:
  //  this is a termination condition
  //  N == sizeof...(TupleTypes)
  //
  template <size_t Idx, typename... TupleTypes>
  inline typename enable_if<Idx == sizeof...(TupleTypes), void>::type
  hash_combine_tup(size_t& , const tuple<TupleTypes...>& ) const {}

  //  this is the computation function
  //  continues till condition N < sizeof...(TupleTypes) holds
  //
  template <size_t Idx, typename... TupleTypes>
      inline typename enable_if <
      Idx<sizeof...(TupleTypes), void>::type hash_combine_tup(
          size_t& seed, const tuple<TupleTypes...>& tup) const {
    hash_combine(seed, get<Idx>(tup));

    //  on to next element
    hash_combine_tup<Idx + 1>(seed, tup);
  }

 public:
  size_t operator()(const tuple<TupleArgs...>& tupleValue) const {
    size_t seed = 0;
    //  begin with the first iteration
    hash_combine_tup<0>(seed, tupleValue);
    return seed;
  }
};

}  // namespace std
#endif
