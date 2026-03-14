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

  class DiscardCombiner
  {
  public:
    using result_type = void;

    template<typename U>
    void combine([[maybe_unused]]U item) {}

    result_type result() {}
  };

  // TODO : demander pour T = void
  template <typename T>
  class LastCombiner
  {
  public:
    using result_type = T;

    template <typename U>
    void combine(U item)
    {
      if constexpr (std::is_same_v<result_type, void>)
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
      if constexpr (std::is_same_v<result_type, void>)
      {
        return;
      }
      else
      {
        return res;
      }
    }

  private:
    std::conditional_t<std::is_same_v<result_type, void>, int, result_type> res;
  };

  template <typename T>
  class VectorCombiner
  {
  public:
    using result_type = std::vector<T>;

    template <typename U>
    void combine(U item)
    {
      res.push_back(static_cast<T>(item));
    }

    result_type result()
    {
      result_type return_val(res);
      res.clear();
      return return_val;
    }

  private:
    result_type res;
  };

  enum class PredicateType
  {
    Unary,
    Binary,
  };

  template <typename T, PredicateType PType = PredicateType::Binary>
  class PredicateCombiner
  {
  public:
    using result_type = std::optional<T>;
    using predicate_type = std::conditional_t<PType == PredicateType::Unary, std::function<bool(T)>, std::function<bool(T, T)>>;

    PredicateCombiner(predicate_type predicate): predicate(std::move(predicate)) {}

    template<typename U>
    void combine(U item) {
      auto n_item = static_cast<T>(item);
      if constexpr (PType == PredicateType::Binary) {
        if (!res.has_value() || predicate(n_item, *res)) res = n_item;
      } else {
        if (predicate(n_item)) res = n_item;
      }
    }

    result_type result()
    {
      result_type return_val(res);
      res.reset();
      return return_val;
    }

  private:
    predicate_type predicate;
    result_type res;
  };

  template <typename Signature, typename Combiner = DiscardCombiner>
  class Signal
  {

    Combiner m_combiner;
    std::map<std::size_t, std::function<Signature>> m_functions;
    using Signature_result_type = typename std::function<Signature>::result_type;

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
      if constexpr (std::is_same_v<Signature_result_type, void>)
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
