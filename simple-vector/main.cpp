#include "tests.h"
//#include <cassert>
//#include <numeric>

using namespace std;

int main() {
    Test1();
    Test2();
    TestReserveConstructor();
    TestReserveMethod();

    TestTemporaryObjConstructor();
    TestTemporaryObjOperator();
    TestNamedMoveConstructor();
    TestNamedMoveOperator();
    TestNoncopiableMoveConstructor();
    TestNoncopiablePushBack();
    TestNoncopiableInsert();
    TestNoncopiableErase();
    return 0;
}
