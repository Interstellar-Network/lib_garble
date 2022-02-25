#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "data/garbled_circuit_full_adder.h"
#include "justgarble/justGarble.h"
#include "justgarble/parallel.h"

// Demonstrate some basic assertions.
TEST(GarbleTest, Adder) {
  GarbledCircuit garbled_circuit;
  interstellar::test::GetFullAdder(&garbled_circuit);

  garbled_circuit.garbleCircuit(123456789);

  ParallelGarbledCircuit parallel_garbled_circuit(std::move(garbled_circuit));

  // REF lib_server HEAD detached at 2ce062c1
  // BOOST_AUTO_TEST_CASE(adder_evaluate_nopackmsg)
  // modification:
  // srand_sse(time(NULL)); move to start of function
  // and parameter added: srand_sse(seed)
  // with seed=123456789

  EXPECT_EQ(parallel_garbled_circuit.nb_inputs_, 3);   // n
  EXPECT_EQ(parallel_garbled_circuit.nb_outputs_, 2);  // m
  EXPECT_EQ(parallel_garbled_circuit.nb_gates_, 9);    // q
  EXPECT_EQ(parallel_garbled_circuit.nb_wires_, 14);   // 4
  EXPECT_EQ(parallel_garbled_circuit.nb_layers_, 4);
  EXPECT_EQ(parallel_garbled_circuit.non_xor_count_, 7);
  ASSERT_THAT(parallel_garbled_circuit.layer_counts_,
              testing::ElementsAre(0, 6, 2, 1));
  ASSERT_THAT(parallel_garbled_circuit.layer_nonxor_counts_,
              testing::ElementsAre(0, 5, 1, 1));
  // TODO wires? not really needed b/c output_labels/intput_labels are
  // constructed from them
  // NOTE: they are NOT a class member so would need a bit of refacto to access
  // those(probably just add a *wires param to "garbleCircuit" above(TEST ONLY))
  /*
garbledCircuit->wires[0]
{...}
label0:
val:
[0]: -1962892466424197958
[1]: 918486782330322410
label1:
val:
[0]: 4903945833004128699
[1]: -8326591785024378746

garbledCircuit->wires[1]
label0:
val:
[0]: -6888441597991027314
[1]: -3297918480307080808
label1:
val:
[0]: 48239628824701071
[1]: 5978087197633848564

garbledCircuit->wires[2]
label0:
val:
[0]: 6362858360273572561
[1]: -6791273187812927717
label1:
val:
[0]: -540046556532701232
[1]: 2381721663782355575

garbledCircuit->wires[3]
label0
val
[0]:-1533876837803754143
[1]:-1594819706762806944
label1
val
[0]:-1038790581825298848
[1]:-1260011222746473767

garbledCircuit->wires[4]
label0
val
[0]:-7478702853685230066
[1]:1732788682471117543
label1
val
[0]:7879439896345833743
[1]:1624802590099678492

garbledCircuit->wires[5]
val
[0]:-6944189940090836427
[1]:-1106158144214293671
label1
val
[0]:-8922668893779160012
[1]:545528198373203220

garbledCircuit->wires[6]
label0
val
[0]:-1306354414474815978
[1]:-123023173653923916
label1
val
[0]:9189648704068833559
[1]:7857134107373711515

garbledCircuit->wires[7]
label0
val
[0]:4946414120322117940
[1]:-2412682274028319630
label1
val
[0]:-1988051041931779019
[1]:6794085202503538974

garbledCircuit->wires[8]
label0
val
[0]:2083012241442841573
[1]:9170704627923917673
label1
val
[0]:-4889660236912871708
[1]:-33216818324664827

garbledCircuit->wires[9]
label0
val
[0]:7693062129551165111
[1]:-6265412512997740380
label1
val
[0]:2641270572566254774
[1]:304960121265660561

garbledCircuit->wires[10]
label0
val
[0]:-2431327605384233256
[1]:-1718692333667175413
label1
val
[0]:5582710977257030873
[1]:-619420934193197476

garbledCircuit->wires[11]
label0
val
[0]:-8303837594409181909
[1]:-3864978066559725843
label1
val
[0]:-8377592127153541590
[1]:9079165760442383716

garbledCircuit->wires[12]
label0
val
[0]:3584010548326071250
[1]:-592280432168791807
label1
val
[0]:-5782547855556280877
[1]:-7032035204244727990

garbledCircuit->wires[13]
label0
val
[0]:9210925814211109780
[1]:-4804654539879680846
label1
val
[0]:5057166663319149717
[1]:5055986645008468911
  */
  ASSERT_THAT(parallel_garbled_circuit.input_labels_,
              testing::ElementsAre(
                  Block(918486782330322410ULL, -1962892466424197958ULL),
                  Block(-8326591785024378746ULL, 4903945833004128699ULL),
                  Block(-3297918480307080808ULL, -6888441597991027314ULL),
                  Block(5978087197633848564ULL, 48239628824701071ULL),
                  Block(-6791273187812927717ULL, 6362858360273572561ULL),
                  Block(2381721663782355575ULL, -540046556532701232ULL),
                  Block(-1594819706762806944ULL, -1533876837803754143ULL),
                  Block(-1260011222746473767ULL, -1038790581825298848ULL)));
  ASSERT_THAT(parallel_garbled_circuit.output_labels_,
              testing::ElementsAre(
                  Block(9170704627923917673ULL, 2083012241442841573ULL),
                  Block(-33216818324664827ULL, -4889660236912871708ULL),
                  Block(-3864978066559725843ULL, -8303837594409181909ULL),
                  Block(9079165760442383716ULL, -8377592127153541590ULL)));
  /*
  For reference: input0,input1,output,isxor
    0 0 3 NONXOR
    0 0 4 NONXOR
    0 0 5 NONXOR
    0 0 6 NONXOR
    0 1 9 NONXOR
    0 1 7 XOR
    2 7 10 NONXOR
    2 7 8 XOR
    9 10 11 NONXOR
  */
  ASSERT_THAT(parallel_garbled_circuit.garbled_gates_,
              testing::ElementsAre(3ULL, 4ULL, 5ULL, 6ULL, 2097161ULL,
                                   9223372036856872967ULL, 8796107702282ULL,
                                   9223380832962478088ULL, 39582439571467ULL));
  // TODO garbledTable
  ASSERT_THAT(parallel_garbled_circuit.outputs_, testing::ElementsAre(8, 11));
  // WARNING: WILL NOT match OLD/REF b/c aes_key and random  are handled a bit
  // differently
  EXPECT_EQ(parallel_garbled_circuit.global_key_,
            Block(2583361968018116390ULL, -1439120098699161818LL));
}