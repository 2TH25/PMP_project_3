#ifndef SIGNAL_H
#define SIGNAL_H

#include <functional>
#include <optional>
#include <vector>
#include <utility>
#include <map>

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

  template <typename T>
  class LastCombiner
  {
  public:
    using result_type = T;

    template<typename U>
    void combine(U item) {
      res = item;
    }

    result_type result() {
      return res;
    }

  private:
    result_type res;
  };

  template <typename T>
  class VectorCombiner
  {
  public:
    using result_type = std::vector<T>;

    VectorCombiner(): res() {}

    template<typename U>
    void combine(U item) {
      res.emplace_back(item);
    }

    result_type result() {
      return res;
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
      if constexpr (PType == PredicateType::Binary) {
        if (!res.has_value() || predicate(*res, item)) res = item;
      } else {
        if (predicate(item)) res = item;
      }
    }

    result_type result() {
      return res;
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

  public:
    using combiner_type = Combiner;

    using result_type = Combiner::result_type;

    Signal(Combiner combiner = Combiner())
        : m_combiner(combiner) {}

    template <typename... CombinerArgs>
    Signal(CombinerArgs... args)
        : m_combiner(args) {}

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
    result_type emitSignal(Args)
    {
      for (auto fun : m_functions)
        m_combiner.combine(m_functions(Args));

      return m_combiner.result();
    }
  };

}

#endif // SIGNAL_H
