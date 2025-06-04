
#include "dmdatetime.h"
#include "gtest.h"

class env_dmdatetime
{
public:
    void init(){}
    void uninit(){}
};

class frame_dmdatetime : public testing::Test
{
public:
    virtual void SetUp()
    {
        env.init();
    }
    virtual void TearDown()
    {
        env.uninit();
    }
protected:
    env_dmdatetime env;
};

TEST_F(frame_dmdatetime, init)
{
    Idmdatetime* module = dmdatetimeGetModule();
    if (module)
    {
        module->Test();
        module->Release();
    }
}
