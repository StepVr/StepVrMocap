# StepVrMocap
StepVr 对接 Unreal 全身动捕+手部捕捉+面部捕捉得插件

# 配置
需要STEPVR_MOCAP服务，并确保动补服务成功启动

# 使用流程
1  启动C:\STEPVR_MOCAP服务，确保服务正常运行
2  开启动捕管理端进行TPose
3  开启游戏内容，链接，展示

# SKT
1 骨骼与提供得基础骨骼结构不同，需要联系AE生成该骨骼得skt
2 位于文件夹Plugins\StepVrMocap\ThirdParty\skt中，将对应得骨骼得skt放入该文件夹中，名字相同，成对使用
3 在骨骼得动画蓝图中，选择StepStream，在右边详细信息中设置该骨骼使用skt得名字

