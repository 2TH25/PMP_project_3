#ifndef SIGNAL_H
#define SIGNAL_H

#include <functional>
#include <optional>
#include <vector>
#include <utility>
#include <map>
#include <iostream>

namespace sig
{

  namespace details
  {
    // Type for result attribute if combiner result_type is void
    template <typename result_type>
    using IsNotVoid = std::conditional_t<std::is_void_v<result_type>, int, result_type>;
  }

  /*===============================================================
                            DiscardCombiner
  ================================================================*/

  class DiscardCombiner
  {
  public:
    using result_type = void; // this combiner return nothing 

    template <typename U>
    void combine([[maybe_unused]] U item) {}

    result_type result() {}
  };

  /*===============================================================
                            LastCombiner
  ================================================================*/

  template <typename T>
  class LastCombiner
  {
  public:
    using result_type = T;

    template <typename U>
    void combine(U item)
    {
      // if the result_type is void, we don't need to save the item
      if constexpr (std::is_void_v<result_type>)
      {
        return;
      }
      else
      {
        res = static_cast<result_type>(item); // it's because U != T
      }
    }

    result_type result()
    {
      // we can't return res if result_type is void because in this case the type of res is int
      if constexpr (std::is_void_v<result_type>)
      {
        return;
      }
      else
      {
        return res;
      }
    }

  private:
    details::IsNotVoid<result_type> res;
  };

  /*===============================================================
                            VectorCombiner
  ================================================================*/

  template <typename T>
  class VectorCombiner
  {
  public:
    using result_type = std::conditional_t<std::is_void_v<T>, void, std::vector<T>>; // it's impossible to have std::vector<void> type

    template <typename U>
    void combine(U item)
    {
      if constexpr (std::is_void_v<result_type>)
      {
        return;
      }
      else
      {
        res.push_back(static_cast<T>(item));
      }
    }

    result_type result()
    {
      if constexpr (std::is_void_v<result_type>)
      {
        return;
      }
      else
      {
        result_type return_val(res);
        res.clear(); // we clear the vector for the case where we us emitSignal a second time

        /* In the case where the old result needed to be deleted from the signal, simply replace m_combiner in Signal with a unique_ptr */

        return return_val;
      }
    }

  private:
    details::IsNotVoid<result_type> res;
  };

  /*===============================================================
                            PredicateType
  ================================================================*/

  enum class PredicateType
  {
    Unary,
    Binary,
  };

  namespace details
  {
    // templated struct to have the case of void result_type in PredicateCombiner for the predicate_type
    template <typename T, PredicateType Ptype>
    struct SwitchPredicateType;

    template <typename T>
    struct SwitchPredicateType<T, PredicateType::Unary>
    {
      using type = typename std::function<bool(T)>;
    };

    template <typename T>
    struct SwitchPredicateType<T, PredicateType::Binary>
    {
      using type = typename std::function<bool(T, T)>;
    };

    // we need two templated struct to avoid any ambiguous template instantiation error
    template <typename T, PredicateType PType>
    struct VoidPredicate
    {
      using type = typename SwitchPredicateType<T, PType>::type;
    };

    template <PredicateType PType>
    struct VoidPredicate<void, PType>
    {
      using type = typename std::function<bool()>;
    };

    // this type return the good type for the predicate thanks to the two preceding struct
    template <typename T, PredicateType PType>
    using predicate_t = typename VoidPredicate<T, PType>::type;
  }

  template <typename T, PredicateType PType = PredicateType::Binary>
  class PredicateCombiner
  {
    using predicate_type = details::predicate_t<T, PType>;

  public:
    using result_type = std::conditional_t<std::is_void_v<T>, void, std::optional<T>>; // it's impossible to have std::optional<void> type

    PredicateCombiner(predicate_type predicate) : predicate(std::move(predicate)) {}

    template <typename U>
    void combine(U item)
    {
      if constexpr (std::is_void_v<result_type>)
      {
        predicate(); // we still execute the predicate
        return;
      }
      else
      {
        auto n_item = static_cast<T>(item);
        if constexpr (PType == PredicateType::Binary) // if it's Binary predicate we compare item and res
        {
          if (!res.has_value() || predicate(n_item, *res))
          {
            res = n_item;
          }
        }
        else
        {
          if (predicate(n_item))
          {
            res = n_item;
          }
        }
      }
    }

    result_type result()
    {
      if constexpr (std::is_void_v<result_type>)
      {
        return;
      }
      else
      {
        result_type return_val(res);
        res.reset();
        return return_val;
      }
    }

  private:
    predicate_type predicate;
    details::IsNotVoid<result_type> res;
  };

  /*===============================================================
                                Signal
  ================================================================*/

  template <typename Signature, typename Combiner = DiscardCombiner>
  class Signal;

  // we use meta template programming to recover Signature_return and Signature_args
  template <typename Signature_return, typename Combiner, typename... Signature_args>
  class Signal<Signature_return(Signature_args...), Combiner>
  {
    Combiner m_combiner; // it's use for creat the return value
    using Signature = Signature_return(Signature_args...);
    std::map<std::size_t, std::function<Signature>> m_functions; // we use map to save functions to be able to delete then with their id

  public:
    using combiner_type = Combiner;
    using result_type = typename Combiner::result_type; 

    // we just save the combiner in m_combiner
    Signal(Combiner combiner = Combiner())
        : m_combiner(combiner) {}

    // this constructor is use for PredicateCombiner or other combiner with arg constructor
    template <typename... CombinerArgs>
    Signal(CombinerArgs... args)
        : m_combiner(args...) {}

    std::size_t connectSlot(std::function<Signature> callback)
    {
      bool is_add(false);
      std::size_t id(0);
      do
      {
        is_add = m_functions.insert({id, callback}).second; // we try to insert the new function in m_functions as long as it fails
        id++;
      } while (!is_add);
      return id - 1;
    }

    void disconnectSlot(std::size_t id)
    {
      m_functions.erase(id); // we just erase the function at the good id 
    }

    result_type emitSignal(Signature_args... args)
    {
      if constexpr (std::is_void_v<Signature_return>) // if the return type of functions is void, we can't do m_combiner.combine(fun(args...))
      {
        for (auto &[id, fun] : m_functions)
        {
          fun(args...);
        }
      }
      else
      {
        for (auto &[id, fun] : m_functions)
        {
          m_combiner.combine(fun(args...));
        }
      }
      return m_combiner.result(); // it doesn't matter if the return type of m_combiner.result() is void
    }
  };

}

#endif // SIGNAL_H
