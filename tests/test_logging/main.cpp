// this is just for experiment/test on-going functions/classes

#include <easy3d/util/logging.h>
#include <easy3d/core/types.h>
#include <easy3d/util/file_system.h>

#include <thread>


using namespace easy3d;

void test_conditional_ccasional_logging() {
    for ( int i = 0; i < 10; ++i ) {
      int old_errno = errno;
      errno = i;
      PLOG_EVERY_N(ERROR, 2) << "Plog every 2, iteration " << google::COUNTER;
      errno = old_errno;

      LOG_FIRST_N(ERROR, 3) << "Log first 3, iteration " << google::COUNTER << std::endl;

      LOG_EVERY_N(ERROR, 3) << "Log every 3, iteration " << google::COUNTER << std::endl;
      LOG_EVERY_N(ERROR, 4) << "Log every 4, iteration " << google::COUNTER << std::endl;

      LOG_IF_EVERY_N(WARNING, true, 5) << "Log if every 5, iteration " << google::COUNTER;
      LOG_IF_EVERY_N(WARNING, false, 3)
          << "Log if every 3, iteration " << google::COUNTER;
      LOG_IF_EVERY_N(INFO, true, 1) << "Log if every 1, iteration " << google::COUNTER;
      LOG_IF_EVERY_N(ERROR, (i < 3), 2)
          << "Log if less than 3 every 2, iteration " << google::COUNTER;
    }
}


void MyFunction() {
    LOG(WARNING) << "function [" << __FUNCTION__ << "] executed";
}


int main (int argc, char *argv[])
{
    logging::initialize(argv[0]);

    //------------------------------------------------

    CHECK_EQ(argv[0], file_system::executable());

    //------------------------------------------------

    // CHECK Operation
    CHECK_NE(1, 2) << ": The world must be ending!";
    // Check if it is euqual
    CHECK_EQ(std::string("abc")[1], 'b');

    int a = 1;
    int b = 2;
    int c = 2;

    CHECK_TRUE(b == c) << ": The world must be ending!";
    CHECK_FALSE(a == b) << ": The world must be ending!";

    CHECK_EQ(std::string("abc")[1], 'b');

    LOG_IF(WARNING, a < b) << "Warning, a < b";
    LOG_IF(ERROR, a < b) << "Error, a < b";

    CHECK_TRUE(b == c);
    CHECK_FALSE(a == b);

    //------------------------------------------------

    for (int i=0; i<100; ++i) {
        LOG_FIRST_N(ERROR, 5) << "LOG_FIRST_N(ERROR, 5): " << i;
    }

    //------------------------------------------------

    std::thread t([=]() {
        LOG(WARNING) << "Run in another thread";
    });
    t.detach();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // ---------------------------------

    int *ptr = new int[10];
    CHECK_NOTNULL(ptr);
    DLOG(INFO) << "of [" << __func__ << "]";

    //------------------------------------------------

    MyFunction();

    //------------------------------------------------

    LOG(INFO) << "Now test logging STL containers:";
    std::vector<int> x;
    x.push_back(1);
    x.push_back(2);
    x.push_back(3);
    LOG(INFO) << "std::vector<int>: " << x;

    //------------------------------------------------

    std::vector<vec3> points;
    for(int i=0; i<200; ++i)
        points.push_back(vec3(i));
    LOG(INFO) << "std::vector<vec3>: " << points;

    //------------------------------------------------

    test_conditional_ccasional_logging();

    //------------------------------------------------

    LOG(INFO) << "---------- TEST has succeeded!!!!!!!!!!!!!!!!! ----------";
//    LOG(FATAL) << "You should have seen the program crashed - just a test :-)";

    return EXIT_SUCCESS;
}
