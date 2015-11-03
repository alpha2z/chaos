人工聊天内容发送机器人


chaos 是作为静态库引用的
config 也是作为静态库引用
robotlib 则是作为源码直接编译到可执行文件中


编译过程：
首先去chaos目录下面执行
make clean&&make

然后，
在工程目录下面 
make clean && make



    
内容推送：
{
    Cmd           uint8_t;       -- 请求类型： 0, 请求包，1，返回包
    ConversionID  uint32_t;      -- 会话id
    OpenID        string;        -- 用户id
    ChannelID     uint32_t;      -- 渠道id
    UserType      uint8_t;       -- 用户类型，0，业务员，1，客户
    SendTime      string;        -- 发送时间，格式 2015-10-10 12:12:12
    Content       string;        -- 对话内容
}

会话操作    
{
    opt           uint8_t;       -- 会话操作类型： 0, 打开，1，关闭
    ConversionID  uint32_t;      -- 会话ID
    openID        string;        -- 用户ID
}












