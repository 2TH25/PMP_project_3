#ifndef SIGNAL_H
#define SIGNAL_H

#include <functional>
#include <optional>
#include <vector>

namespace sig {

  class DiscardCombiner {
  public:
    using result_type = void;

    template<typename U>
    void combine(U item) {}

    result_type result() {}
  };

  template<typename T>
  class LastCombiner {
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

  template<typename T>
  class VectorCombiner {
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

  enum class PredicateType {
    Unary,
    Binary,
  };

  template<typename T, PredicateType PType = PredicateType::Binary>
  class PredicateCombiner {
  public:
    using result_type = std::optional<T>;
    using predicate_type = std::conditional_t<PType == PredicateType::Unary, std::function<bool(T)>, std::function<bool(T, T)>>;

    PredicateCombiner(std::function<predicate_type> predicate): predicate(predicate) {}

    template<typename U>
    void combine(U item) {
      if constexpr (PType == PredicateType::Binary) {
        if (!res.has_value()) {
          res = item;
        } else if (predicate(*res, item)) {
          res = item;
        }
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

  template<typename Signature, typename Combiner = DiscardCombiner>
  class Signal {
  public:
    using combiner_type = /* implementation defined */;

    using result_type = /* implementation defined */;

    Signal(Combiner combiner = Combiner()) {
      // implementation defined
    }

    template<typename... CombinerArgs>
    Signal(CombinerArgs ... args) {
      // implementation defined
    }

    std::size_t connectSlot(std::function<Signature> callback) {
      // implementation defined
    }

    void disconnectSlot(std::size_t id) {
      // implementation defined
    }

    result_type emitSignal(/* implementation defined */) {
      // implementation defined
    }
  };

}

#endif // SIGNAL_H
