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

TEST(test, test7)
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

TEST(test, test2)
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

TEST(test, test3)
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

TEST(test, test4)
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

TEST(test, test5)
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

TEST(test, test8)
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

// TODO : demander si possible
TEST(test, test9)
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

// TODO : demander si possible
TEST(test, test10)
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

TEST(test, test6)
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

TEST(test, test)
{
  sig::Signal<void(int)> sig;
  // connect a simple function
  sig.connectSlot(callback);
  // connect a lambda function
  int res = 0;
  sig.connectSlot([&res](int x)
                  { res = x; });
  // emit the signal
  sig.emitSignal(1);
  // here res equals 1 and "Hello 1" is printed on stdout
  assert(res == 1);
  sig::Signal<int(int), sig::PredicateCombiner<int>> sig2([](int lhs, int rhs) -> bool
                                                          { return lhs < rhs; });
  sig2.connectSlot([](int x)
                   { return x; });
  sig2.connectSlot([](int x)
                   { return x * x; });
  std::size_t id = sig2.connectSlot([](int x)
                                    { return x * -1; });
  auto result = sig2.emitSignal(8);
  assert(result); // has a value
  assert(*result == -8);
  sig2.disconnectSlot(id);
  result = sig2.emitSignal(8);
  assert(result); // has a value
  assert(*result == 8);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
