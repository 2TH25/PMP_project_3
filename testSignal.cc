#include "Signal.h"

#include <gtest/gtest.h>

void callback(int param)
{
  std::printf("Hello %i\n", param);
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
