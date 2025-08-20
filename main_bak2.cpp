//// ============= "以下内容仅测试解析包的流程是否正确，不作为程序构成 ============="

//#include <gtest/gtest.h>
//#include <gmock/gmock.h>
//#include <QFileDialog>
//#include <QSignalSpy>
//#include "playercore.h"
//#include <fstream>
//#include <memory>

//// 测试用临时文件创建工具
//class TempFileHelper {
//public:
//    static std::string createTempFile(const std::string& content) {
//        std::string filename = "test_video_";
//        filename += std::to_string(rand()) + ".mp4";
//        std::ofstream f(filename);
//        f << content;
//        f.close();
//        return filename;
//    }

//    static void deleteTempFile(const std::string& filename) {
//        std::remove(filename.c_str());
//    }
//};

//// 测试文件选择功能
//TEST(PlayerCoreTest, FileSelection) {
//    PlayerCore player;

//    // 测试选择无效文件
//    EXPECT_FALSE(player.openFile("invalid_non_existent_file.mp4"));

//    // 测试选择空路径
//    EXPECT_FALSE(player.openFile(""));

//    // 测试选择合法文件（使用实际测试视频）
//    std::string validFile = "test_sample.mp4"; // 请替换为实际测试文件
//    EXPECT_TRUE(player.openFile(validFile.c_str()));
//}

//// 测试文件解析流程
//TEST(PlayerCoreTest, FileParsing) {
//    PlayerCore player;
//    std::string testFile = TempFileHelper::createTempFile("dummy video data");

//    // 测试解析无效文件（非视频格式）
//    EXPECT_TRUE(player.openFile(testFile.c_str())); // 打开文件本身成功
//    EXPECT_EQ(player.getVideoStreamIndex(), -1);    // 但无法找到视频流
//    EXPECT_EQ(player.getAudioStreamIndex(), -1);    // 无法找到音频流

//    TempFileHelper::deleteTempFile(testFile);

//    // 测试解析合法视频文件
//    std::string validFile = "test_sample.mp4"; // 请替换为实际测试文件
//    EXPECT_TRUE(player.openFile(validFile.c_str()));
//    EXPECT_NE(player.getVideoStreamIndex(), -1);   // 视频流索引有效
//    EXPECT_NE(player.getAudioStreamIndex(), -1);   // 音频流索引有效
//    EXPECT_GT(player.getTotalDuration(), 0);       // 总时长有效
//    EXPECT_GT(player.getFrameDuration(), 0);       // 帧间隔有效
//}

//// 测试解码循环基础功能（使用Mock验证流程）
//class MockPlayerCore : public PlayerCore {
//public:
//    MOCK_METHOD(void, log_info, (const char*), (override));
//    MOCK_METHOD(void, emitError, (const QString&), ());

//    // 重写信号发射函数便于测试
//    void errorOccurred(const QString& msg) override {
//        emitError(msg);
//    }
//};

//TEST(PlayerCoreDecodeTest, BasicDecodeLoop) {
//    MockPlayerCore player;
//    std::string validFile = "test_sample.mp4"; // 请替换为实际测试文件
//    ASSERT_TRUE(player.openFile(validFile.c_str()));

//    // 验证初始状态
//    EXPECT_FALSE(player.isEOF());
//    EXPECT_FALSE(player.isPlaying());

//    // 启动播放并验证解码循环启动
//    player.play();
//    EXPECT_TRUE(player.isPlaying());

//    // 等待解码循环运行一段时间（1秒）
//    std::this_thread::sleep_for(std::chrono::seconds(1));

//    // 验证至少解析到一些帧
//    EXPECT_GT(player.getFrameCount(), 0); // 需要在PlayerCore中添加getFrameCount()方法

//    // 停止播放
//    player.stop();
//    EXPECT_FALSE(player.isPlaying());
//}

//// 测试重复帧处理逻辑
//TEST(PlayerCoreDecodeTest, DuplicateFrameHandling) {
//    MockPlayerCore player;
//    std::string validFile = "test_duplicate_frames.mp4"; // 含重复帧的测试文件
//    ASSERT_TRUE(player.openFile(validFile.c_str()));

//    // 期望重复帧被正确过滤
//    EXPECT_CALL(player, log_info(testing::ContainsRegex("重复帧，跳过：PTS=.*"))).Times(testing::AtLeast(1));

//    player.play();
//    std::this_thread::sleep_for(std::chrono::seconds(2));
//    player.stop();
//}

//// 测试异常场景（文件损坏）
//TEST(PlayerCoreDecodeTest, CorruptedFileHandling) {
//    MockPlayerCore player;
//    std::string corruptedFile = TempFileHelper::createTempFile("corrupted data");
//    ASSERT_TRUE(player.openFile(corruptedFile.c_str()));

//    // 期望解码过程中出现错误
//    EXPECT_CALL(player, emitError(testing::ContainsRegex("发送数据包到解码器失败"))).Times(testing::AtLeast(1));

//    player.play();
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    player.stop();

//    TempFileHelper::deleteTempFile(corruptedFile);
//}

//int main(int argc, char **argv) {
//    testing::InitGoogleTest(&argc, argv);
//    // 初始化Qt环境（如需处理信号和槽）
//    QCoreApplication app(argc, argv);
//    return RUN_ALL_TESTS();
//}
