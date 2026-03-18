#include "Signal.h"

#include <gtest/gtest.h>

void callback(int param)
{
  std::printf("Hello %i\n", param);
}

class TestCombiner
{
public:
  using result_type = int;
  ;

  TestCombiner(int x, int y)
      : x(x),
        y(y) {}

  template <typename U>
  void combine([[maybe_unused]] U item)
  {
    auto n_item = static_cast<int>(item);
    if (it_is_a_res_wth_a_strange_name < n_item)
      it_is_a_res_wth_a_strange_name = n_item;
  }

  result_type result()
  {
    it_is_a_res_wth_a_strange_name = it_is_a_res_wth_a_strange_name * x - y;
    return it_is_a_res_wth_a_strange_name;
  }

private:
  double it_is_a_res_wth_a_strange_name = 0;
  int x;
  int y;
};

TEST(DiscardCombiner, nomal)
{
  sig::DiscardCombiner com;

  EXPECT_TRUE(std::is_void_v<sig::DiscardCombiner::result_type>);

  com.combine(1);
  com.combine(1.56f);
  com.combine(std::vector{1, 2, 4});
  com.result();
}

TEST(LastCombiner, nomal)
{
  sig::LastCombiner<int> com;

  EXPECT_TRUE(std::is_integral_v<sig::LastCombiner<int>::result_type>);

  com.combine(1);
  com.combine(1.56f);
  com.combine('c');
  auto res = com.result();
  EXPECT_EQ(res, 'c');

  com.combine(1);
  com.combine(1.56f);
  res = com.result();
  EXPECT_EQ(res, 1);
}

TEST(LastCombiner, void_type)
{
  sig::LastCombiner<void> com;

  EXPECT_TRUE(std::is_void_v<sig::LastCombiner<void>::result_type>);

  com.combine(1);
  com.combine(1.56f);
  com.combine('c');
  com.result();
}

TEST(VectorCombiner, nomal)
{
  sig::VectorCombiner<int> com;

  bool is_same = std::is_same_v<sig::VectorCombiner<int>::result_type, std::vector<int>>;
  EXPECT_TRUE(is_same);

  com.combine(1);
  com.combine(1.56f);
  com.combine('c');
  auto res = com.result();
  std::vector<int> test{1, 1, 'c'};
  EXPECT_EQ(res, test);

  com.combine(1);
  com.combine(1.56f);
  res = com.result();
  test = {1, 1};
  EXPECT_EQ(res, test);
}

TEST(VectorCombiner, empty_result)
{
  sig::VectorCombiner<int> com;

  auto res = com.result();
  EXPECT_TRUE(res.empty());
}

TEST(VectorCombiner, void_type)
{
  sig::VectorCombiner<void> com;

  bool is_same = std::is_void_v<sig::VectorCombiner<void>::result_type>;
  EXPECT_TRUE(is_same);

  com.combine(1);
  com.combine(1.56f);
  com.combine('c');
  com.result();
}

TEST(PredicateCombiner, nomal_B)
{
  int compt(0);
  sig::PredicateCombiner<int> com([&compt](int a, int b) -> bool
                                  { compt+=1;
                                    return a > b; });

  bool is_same = std::is_same_v<sig::PredicateCombiner<int>::result_type, std::optional<int>>;
  EXPECT_TRUE(is_same);

  com.combine(1);
  com.combine(1000.54f);
  com.combine('c');
  auto res = com.result();
  EXPECT_TRUE(res.has_value());
  EXPECT_EQ(res, 1000);
  EXPECT_EQ(compt, 2);

  com.combine(1);
  com.combine('c');
  res = com.result();
  EXPECT_TRUE(res.has_value());
  EXPECT_EQ(res, 'c');
  EXPECT_EQ(compt, 3);
}

TEST(PredicateCombiner, no_result_B)
{
  int compt(0);
  sig::PredicateCombiner<int> com([&compt]([[maybe_unused]] int a, [[maybe_unused]] int b) -> bool
                                  { compt+=1;
                                    return compt > 10; });

  com.combine(1);
  com.combine(1000.54f);
  com.combine('c');
  auto res = com.result();
  EXPECT_TRUE(res.has_value());
  EXPECT_EQ(res, 1);
  EXPECT_EQ(compt, 2);

  com.combine(1);
  com.combine('c');
  res = com.result();
  EXPECT_TRUE(res.has_value());
  EXPECT_EQ(res, 1);
  EXPECT_EQ(compt, 3);
}

TEST(PredicateCombiner, void_type_B)
{
  int compt(0);
  sig::PredicateCombiner<void> com([&compt]() -> bool
                                   { compt+=1;
                                    return true; });

  bool is_same = std::is_void_v<sig::PredicateCombiner<void>::result_type>;
  EXPECT_TRUE(is_same);

  com.combine(1);
  com.combine(1000.54f);
  com.combine('c');
  com.result();
  EXPECT_EQ(compt, 3);

  com.combine(1);
  com.combine('c');
  EXPECT_EQ(compt, 5);
}

TEST(PredicateCombiner, nomal_U)
{
  int compt(0);
  sig::PredicateCombiner<int, sig::PredicateType::Unary> com([&compt](int a) -> bool
                                                             {compt+=1;
                                                              return a > compt; });

  bool is_same = std::is_same_v<sig::PredicateCombiner<int, sig::PredicateType::Unary>::result_type, std::optional<int>>;
  EXPECT_TRUE(is_same);

  com.combine(1);
  com.combine(-1000.54f);
  com.combine('c');
  auto res = com.result();
  EXPECT_TRUE(res.has_value());
  EXPECT_EQ(res, 'c');
  EXPECT_EQ(compt, 3);

  com.combine(1);
  com.combine(1000.54f);
  res = com.result();
  EXPECT_TRUE(res.has_value());
  EXPECT_EQ(res, 1000);
  EXPECT_EQ(compt, 5);
}

TEST(PredicateCombiner, no_result_U)
{
  int compt(0);
  sig::PredicateCombiner<int, sig::PredicateType::Unary> com([&compt]([[maybe_unused]] int a) -> bool
                                                             {compt+=1;
                                                              return compt > 10; });

  com.combine(1);
  com.combine(1000.54f);
  com.combine('c');
  auto res = com.result();
  EXPECT_FALSE(res.has_value());
  EXPECT_EQ(compt, 3);

  com.combine(1);
  com.combine('c');
  res = com.result();
  EXPECT_FALSE(res.has_value());
  EXPECT_EQ(compt, 5);
}

TEST(PredicateCombiner, void_type_U)
{
  int compt(0);
  sig::PredicateCombiner<void, sig::PredicateType::Unary> com([&compt]() -> bool
                                                              { compt+=1;
                                                                return true; });

  bool is_same = std::is_void_v<sig::PredicateCombiner<void, sig::PredicateType::Unary>::result_type>;
  EXPECT_TRUE(is_same);

  com.combine(1);
  com.combine(1000.54f);
  com.combine('c');
  com.result();
  EXPECT_EQ(compt, 3);

  com.combine(1);
  com.combine('c');
  EXPECT_EQ(compt, 5);
}

// TODO : faire un test avec des times

TEST(Signal_DiscardCombiner, normal)
{
  sig::Signal<void(int)> sig;

  int res = 0;
  sig.connectSlot([&res](int x)
                  { res = x; });

  sig.connectSlot([&res](int x)
                  { res = x + 2; });

  std::size_t id = sig.connectSlot([&res](int x)
                                   { res = x + 5; });

  sig.emitSignal(1);
  EXPECT_EQ(res, 6);

  sig.disconnectSlot(id);
  sig.emitSignal(1);
  EXPECT_EQ(res, 3);
}

TEST(Signal_DiscardCombiner, wth_return_type)
{
  sig::Signal<int(int)> sig;

  int res = 0;
  sig.connectSlot([&res](int x)
                  { res = x;
                    return x; });

  sig.connectSlot([&res](int x)
                  { res = x + 2;
                    return x; });

  std::size_t id = sig.connectSlot([&res](int x)
                                   {res = x + 5;
                                    return x; });

  sig.emitSignal(1);
  EXPECT_EQ(res, 6);

  sig.disconnectSlot(id);
  sig.emitSignal(1);
  EXPECT_EQ(res, 3);
}

TEST(Signal_DiscardCombiner, ptr_tricks)
{
  sig::Signal<int(int *), sig::DiscardCombiner> sig;

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return *x; });

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return *x; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int *x)
                                   { *x += 1; 
                                     return *x; });

  int *test_nb = new int;
  *test_nb = 0;
  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 3);

  sig.disconnectSlot(id);

  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 5);
  delete test_nb;
}

TEST(Signal_LastCombiner, not_same_type)
{
  sig::Signal<int(int, int, int), sig::LastCombiner<char>> sig;

  sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                  { return x; });

  sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                  { return y; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                                   { return z; });

  char res = sig.emitSignal(65, 66, 67);
  EXPECT_EQ(res, 'C');

  sig.disconnectSlot(id);

  res = sig.emitSignal(65, 66, 67);
  EXPECT_EQ(res, 'B');
}

TEST(Signal_LastCombiner, void_combiner_type)
{
  sig::Signal<int(int *), sig::LastCombiner<void>> sig;

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return *x; });

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return *x; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int *x)
                                   { *x += 1; 
                                     return *x; });

  int *test_nb = new int;
  *test_nb = 0;
  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 3);

  sig.disconnectSlot(id);

  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 5);
  delete test_nb;
}

TEST(Signal_LastCombiner, void_signal_type)
{
  sig::Signal<void(int *), sig::LastCombiner<int>> sig;

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return; });

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int *x)
                                   { *x += 1; 
                                     return; });

  int *test_nb = new int;
  *test_nb = 0;
  int res = sig.emitSignal(test_nb);
  res += 1;
  EXPECT_EQ(*test_nb, 3);

  sig.disconnectSlot(id);

  res = sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 5);
  delete test_nb;
}

TEST(Signal_LastCombiner, vector_type)
{
  sig::Signal<std::vector<int>(int, int, int), sig::LastCombiner<std::vector<int>>> sig;

  sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                  { return std::vector<int>{x, y, z}; });

  sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                  { return std::vector<int>{y, z, x}; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                                   { return std::vector<int>{z, x, y}; });

  std::vector<int> test{3, 1, 2};
  auto res = sig.emitSignal(1, 2, 3);
  EXPECT_EQ(res, test);

  sig.disconnectSlot(id);

  test = {2, 3, 1};
  res = sig.emitSignal(1, 2, 3);
  EXPECT_EQ(res, test);
}

TEST(Signal_LastCombiner, ptr_tricks)
{
  sig::Signal<int *(int *), sig::LastCombiner<int *>> sig;

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return x; });

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return x; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int *x)
                                   { *x += 1; 
                                     return x; });

  int *test_nb = new int;
  *test_nb = 0;
  int *res = sig.emitSignal(test_nb);
  EXPECT_EQ(*res, 3);

  sig.disconnectSlot(id);

  res = sig.emitSignal(test_nb);
  EXPECT_EQ(*res, 5);
  *test_nb = 10;
  EXPECT_EQ(*res, 10);
  res = sig.emitSignal(test_nb);
  EXPECT_EQ(*res, 12);
  delete test_nb;
}

TEST(Signal_VectorCombiner, not_same_type)
{
  sig::Signal<int(int, int, int), sig::VectorCombiner<char>> sig;

  sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                  { return x; });

  sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                  { return y; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                                   { return z; });

  auto res = sig.emitSignal(65, 66, 67);
  std::vector<char> test{'A', 'B', 'C'};
  EXPECT_EQ(res, test);

  sig.disconnectSlot(id);

  test = {'A', 'B'};
  res = sig.emitSignal(65, 66, 67);
  EXPECT_EQ(res, test);
}

TEST(Signal_VectorCombiner, void_combiner_type)
{
  sig::Signal<int(int *), sig::VectorCombiner<void>> sig;

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return *x; });

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return *x; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int *x)
                                   { *x += 1; 
                                     return *x; });

  int *test_nb = new int;
  *test_nb = 0;
  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 3);

  sig.disconnectSlot(id);

  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 5);
  delete test_nb;
}

TEST(Signal_VectorCombiner, void_signal_type)
{
  sig::Signal<void(int *), sig::VectorCombiner<int>> sig;

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return; });

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int *x)
                                   { *x += 1; 
                                     return; });

  int *test_nb = new int;
  *test_nb = 0;
  auto res = sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 3);
  EXPECT_TRUE(res.empty());

  sig.disconnectSlot(id);

  res = sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 5);
  EXPECT_TRUE(res.empty());
  delete test_nb;
}

TEST(Signal_PredicateCombiner, void_combiner_type_U)
{
  float *test_nb = new float;
  sig::Signal<float *(float *), sig::PredicateCombiner<void, sig::PredicateType::Unary>> sig([test_nb]()
                                                                                             {if (*test_nb < 81) { *test_nb /= 5; } 
                                                                                              return true; });

  sig.connectSlot([]([[maybe_unused]] float *x)
                  { *x *= 10;
                    return x; });

  sig.connectSlot([]([[maybe_unused]] float *x)
                  { *x *= 10;
                    return x; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] float *x)
                                   { *x *= 10;
                                     return x; });

  *test_nb = 0.5f;
  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 4.0f);

  sig.disconnectSlot(id);

  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 16);
  delete test_nb;
}

TEST(Signal_PredicateCombiner, void_signal_type_U)
{
  int *test_nb = new int;
  sig::Signal<void(int *), sig::PredicateCombiner<int, sig::PredicateType::Unary>> sig([test_nb]([[maybe_unused]] int a)
                                                                                       {*test_nb += 1; 
                                                                                              return true; });

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return; });

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int *x)
                                   { *x += 1; 
                                     return; });

  *test_nb = 0;
  auto res = sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 3);
  EXPECT_FALSE(res.has_value());

  sig.disconnectSlot(id);

  res = sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 5);
  EXPECT_FALSE(res.has_value());
  delete test_nb;
}

TEST(Signal_PredicateCombiner, ptr_tricks_U)
{
  sig::Signal<float *(float *), sig::PredicateCombiner<float *, sig::PredicateType::Unary>> sig([](float *d1)
                                                                                                { if (*d1 < 81) { *d1 /= 5; } 
                                                                                                  return true; });

  sig.connectSlot([]([[maybe_unused]] float *x)
                  { *x *= 10;
                    return x; });

  sig.connectSlot([]([[maybe_unused]] float *x)
                  { *x *= 10;
                    return x; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] float *x)
                                   { *x *= 10;
                                     return x; });

  float *test_nb = new float;
  *test_nb = 0.5f;
  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 4.0f);

  sig.disconnectSlot(id);

  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 16);
  delete test_nb;
}

TEST(Signal_PredicateCombiner, ptr_tricks_U_2)
{
  sig::Signal<int *(int *), sig::PredicateCombiner<int *, sig::PredicateType::Unary>> sig([](int *x) -> bool
                                                                                          { *x += 1;
                                                                                            return true; });

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return x; });

  sig.connectSlot([]([[maybe_unused]] int *x)
                  { *x += 1;
                    return x; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int *x)
                                   { *x += 1; 
                                     return x; });

  int *test_nb = new int;
  *test_nb = 0;
  auto res = sig.emitSignal(test_nb);
  EXPECT_EQ(**res, 6);

  sig.disconnectSlot(id);

  res = sig.emitSignal(test_nb);
  EXPECT_EQ(**res, 10);
  EXPECT_EQ(*test_nb, 10);
  *test_nb = 2;
  EXPECT_EQ(**res, 2);
  delete test_nb;
}

TEST(Signal_PredicateCombiner, void_type_B)
{
  float *test_nb = new float;
  sig::Signal<float *(float *), sig::PredicateCombiner<void, sig::PredicateType::Binary>> sig([test_nb]()
                                                                                              { if (*test_nb < 81) { *test_nb /= 5; } 
                                                                                                return true; });

  sig.connectSlot([]([[maybe_unused]] float *x)
                  { *x *= 10;
                    return x; });

  sig.connectSlot([]([[maybe_unused]] float *x)
                  { *x *= 10;
                    return x; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] float *x)
                                   { *x *= 10;
                                     return x; });

  *test_nb = 0.5f;
  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 4.0f);

  sig.disconnectSlot(id);

  sig.emitSignal(test_nb);
  EXPECT_EQ(*test_nb, 16);
  delete test_nb;
}

TEST(Signal, personal_combiner)
{
  sig::Signal<int(int, int, int), TestCombiner> sig(10, 5);

  sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                  { return x; });

  sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                  { return y; });

  std::size_t id = sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                                   { return z; });

  int res = sig.emitSignal(1, 2, 3);
  EXPECT_EQ(res, 25);

  sig.disconnectSlot(id);

  sig.connectSlot([]([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int z)
                  { return z + 2; });

  res = sig.emitSignal(1, 2, 3);
  EXPECT_EQ(res, 245);
}

// TODO : ajouter test avec objet non copiable
// TEST(test, test14)
// {
//   sig::Signal<std::unique_ptr<int>(std::unique_ptr<int>), sig::VectorCombiner<std::unique_ptr<int>>> sig;

//   sig.connectSlot([]([[maybe_unused]] std::unique_ptr<int> x)
//                   { *x += 1;
//                     return x; });

//   sig.connectSlot([]([[maybe_unused]] std::unique_ptr<int> x)
//                   { *x += 1;
//                     return x; });

//   std::size_t id = sig.connectSlot([]([[maybe_unused]] std::unique_ptr<int> x)
//                                    { *x += 1;
//                                      return x; });

//   auto test_nb = std::make_unique<int>();
//   *test_nb = 0;
//   sig.emitSignal(std::move(test_nb));
//   EXPECT_EQ(*test_nb, 3);

//   sig.disconnectSlot(id);

//   sig.emitSignal(std::move(test_nb));
//   EXPECT_EQ(*test_nb, 5);
// }

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
