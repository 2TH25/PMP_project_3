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
    template <typename result_type>
    using IsNotVoid = std::conditional_t<std::is_void_v<result_type>, int, result_type>;
  }

  class DiscardCombiner
  {
  public:
    using result_type = void;

    template <typename U>
    void combine([[maybe_unused]] U item) {}

    result_type result() {}
  };

  template <typename T>
  class LastCombiner
  {
  public:
    using result_type = T;

    template <typename U>
    void combine(U item)
    {
      if constexpr (std::is_void_v<result_type>)
      {
        return;
      }
      else
      {
        res = static_cast<result_type>(item);
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
        return res;
      }
    }

  private:
    details::IsNotVoid<result_type> res;
  };

  template <typename T>
  class VectorCombiner
  {
  public:
    using result_type = std::conditional_t<std::is_void_v<T>, void, std::vector<T>>;

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
        res.clear();
        return return_val;
      }
    }

  private:
    details::IsNotVoid<result_type> res;
  };

  enum class PredicateType
  {
    Unary,
    Binary,
  };

  namespace details
  {
    template<typename T, PredicateType Ptype> struct SwitchPredicateType;

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

    template <typename T, PredicateType PType>
    using predicate_t = typename VoidPredicate<T, PType>::type;
  }

  template <typename T, PredicateType PType = PredicateType::Binary>
  class PredicateCombiner
  {
  public:
    using result_type = std::conditional_t<std::is_void_v<T>, void, std::optional<T>>;
    using predicate_type = details::predicate_t<T, PType>;

    PredicateCombiner(predicate_type predicate) : predicate(std::move(predicate)) {}

    template <typename U>
    void combine(U item)
    {
      if constexpr (std::is_void_v<result_type>)
      {
        predicate();
        return;
      }
      else
      {
        auto n_item = static_cast<T>(item);
        if constexpr (PType == PredicateType::Binary)
        {
          if (!res.has_value() || predicate(n_item, *res))
            res = n_item;
        }
        else
        {
          if (predicate(n_item))
            res = n_item;
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

  template <typename Signature, typename Combiner = DiscardCombiner>
  class Signal;

  template <typename Signature_return, typename Combiner, typename... Signature_args>
  class Signal<Signature_return(Signature_args...), Combiner>
  {
    Combiner m_combiner;
    using Signature = Signature_return(Signature_args...);
    std::map<std::size_t, std::function<Signature>> m_functions;
    using Signature_result_type = Signature_return;

  public:
    using combiner_type = Combiner;
    using result_type = typename Combiner::result_type;

    Signal(Combiner combiner = Combiner())
        : m_combiner(combiner) {}

    template <typename... CombinerArgs>
    Signal(CombinerArgs... args)
        : m_combiner(args...) {}

    std::size_t connectSlot(std::function<Signature> callback)
    {
      bool is_add(false);
      std::size_t id(0);
      do
      {
        is_add = m_functions.insert({id, callback}).second;
        id++;
      } while (!is_add);
      return id - 1;
    }

    void disconnectSlot(std::size_t id)
    {
      m_functions.erase(id);
    }

    template <typename... Args>
    result_type emitSignal(Args... args)
    {
      if constexpr (std::is_void_v<Signature_result_type>)
        for (auto &[id, fun] : m_functions)
          fun(args...);

      else
        for (auto &[id, fun] : m_functions)
          m_combiner.combine(fun(args...));

      return m_combiner.result();
    }
  };

}

#endif // SIGNAL_H
