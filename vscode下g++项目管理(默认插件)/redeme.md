run.cpp调用了两个模块，其各自有头文件和源文件，在vscode中需要对.vscode中的配置文件修改才能成功编译

## c_cpp_properties.json

添加include路径

![image-20220417140825769](redeme.assets/image-20220417140825769.png)

## AD

在g++运行参数中添加源文件和头文件路径

![image-20220417141006370](redeme.assets/image-20220417141006370.png)



ctrl+b编译即可