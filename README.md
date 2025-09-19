# 温馨提示：首次测试不要直接嵌入哦



## 详细参见：[Apatch内核模块分享(环境隐藏)](https://bbs.kanxue.com/thread-288041.htm)



### Yuuki Parser 命令参考手册

#### 数据类型说明

| 类型ID | 类型名称     | 描述               | 数据格式                  |
| ------ | ------------ | ------------------ | ------------------------- |
| 1      | PATH_MAPPING | 路径映射           | MAP:uid:src_path:dst_path |
| 2      | SINGLE_PATH  | 单个路径(或关键词) | PATH:path                 |
| 3      | PTRACE_UID   | PTRACE用户ID       | PTRACE:uid                |

#### 管理命令

##### 状态查询命令

| 命令         | 功能             | 使用示例     | 返回信息                     |
| ------------ | ---------------- | ------------ | ---------------------------- |
| `CMD:STATUS` | 显示系统状态统计 | `CMD:STATUS` | 显示总条目数和各类型数据统计 |
| `CMD:HELP`   | 显示帮助信息     | `CMD:HELP`   | 显示所有可用命令和格式说明   |

##### 数据显示命令

| 命令              | 功能                   | 使用示例                                                    | 说明                             |
| ----------------- | ---------------------- | ----------------------------------------------------------- | -------------------------------- |
| `CMD:DUMP`        | 打印所有数据到dmesg    | `CMD:DUMP`                                                  | 输出所有类型的所有数据           |
| `CMD:DUMP:TYPE:N` | 打印指定类型的数据     | `CMD:DUMP:TYPE:1`<br>`CMD:DUMP:TYPE:2`<br>`CMD:DUMP:TYPE:3` | N为类型ID (1-3)                  |
| `CMD:LIST:TYPE:N` | 列出指定类型条目及索引 | `CMD:LIST:TYPE:1`<br>`CMD:LIST:TYPE:2`<br>`CMD:LIST:TYPE:3` | 显示类型索引和全局索引的对应关系 |

##### 数据清理命令

| 命令               | 功能             | 使用示例                                                     | 说明                   |
| ------------------ | ---------------- | ------------------------------------------------------------ | ---------------------- |
| `CMD:CLEAR`        | 清空所有数据     | `CMD:CLEAR`                                                  | 删除所有类型的所有数据 |
| `CMD:CLEAR:TYPE:N` | 清空指定类型数据 | `CMD:CLEAR:TYPE:1`<br>`CMD:CLEAR:TYPE:2`<br>`CMD:CLEAR:TYPE:3` | N为类型ID (1-3)        |

##### 数据删除命令

| 命令                                 | 功能                   | 使用示例                                                     | 说明                                |
| ------------------------------------ | ---------------------- | ------------------------------------------------------------ | ----------------------------------- |
| `CMD:REMOVE:INDEX:N`                 | 删除指定全局索引的条目 | `CMD:REMOVE:INDEX:5`<br>`CMD:REMOVE:INDEX:10`                | N为全局索引号                       |
| `CMD:REMOVE:TYPE:type_id:type_index` | 删除指定类型的第N条    | `CMD:REMOVE:TYPE:1:0`<br>`CMD:REMOVE:TYPE:2:3`<br>`CMD:REMOVE:TYPE:3:1` | 删除type_id类型的第type_index条数据 |

#### 数据添加格式

##### 路径映射 (类型1)

```
MAP:uid:src_path:dst_path
```

**示例：**

```
MAP:1000:/data/app:/system/app
MAP:0:/sdcard:/mnt/sdcard
MAP:1001:/data/user/1001:/data/data
```

##### 单个路径 (类型2)

```
PATH:path
```

**示例：**

```
PATH:/system/bin/su
PATH:/data/local/tmp
PATH:/system/etc/hosts
```

##### PTRACE UID (类型3)

```
PTRACE:uid
```

**示例：**

```
PTRACE:1000
PTRACE:0
PTRACE:2000
```

#### 完整使用流程示例

##### 1. 添加数据

```bash
# 添加路径映射
echo "MAP:1000:/data/app:/system/app" > /dev/yuuki_misc

# 添加单个路径
echo "PATH:/system/bin/su" > /dev/yuuki_misc

# 添加PTRACE UID
echo "PTRACE:1000" > /dev/yuuki_misc
```

##### 2. 查看状态

```bash
# 查看整体状态
echo "CMD:STATUS" > /dev/yuuki_misc

# 查看帮助
echo "CMD:HELP" > /dev/yuuki_misc
```

##### 3. 列出数据

```bash
# 列出所有路径映射条目
echo "CMD:LIST:TYPE:1" > /dev/yuuki_misc

# 列出所有单个路径条目  
echo "CMD:LIST:TYPE:2" > /dev/yuuki_misc

# 列出所有PTRACE UID条目
echo "CMD:LIST:TYPE:3" > /dev/yuuki_misc
```

##### 4. 导出数据

```bash
# 导出所有数据
echo "CMD:DUMP" > /dev/yuuki_misc

# 只导出路径映射数据
echo "CMD:DUMP:TYPE:1" > /dev/yuuki_misc
```

##### 5. 删除数据

```bash
# 删除全局索引为5的条目
echo "CMD:REMOVE:INDEX:5" > /dev/yuuki_misc

# 删除路径映射类型的第1条(索引0)
echo "CMD:REMOVE:TYPE:1:0" > /dev/yuuki_misc

# 删除单个路径类型的第2条(索引1)
echo "CMD:REMOVE:TYPE:2:1" > /dev/yuuki_misc
```

##### 6. 清理数据

```bash
# 清空所有路径映射
echo "CMD:CLEAR:TYPE:1" > /dev/yuuki_misc

# 清空所有数据
echo "CMD:CLEAR" > /dev/yuuki_misc
```

#### 错误代码

| 错误代码 | 常量名               | 描述           |
| -------- | -------------------- | -------------- |
| 0        | PARSER_SUCCESS       | 操作成功       |
| -1       | PARSER_ERR_INVALID   | 无效的命令格式 |
| -2       | PARSER_ERR_UNKNOWN   | 未知命令       |
| -3       | PARSER_ERR_PARAM     | 参数错误       |
| -4       | PARSER_ERR_OPERATION | 操作失败       |

#### 注意事项

1. **命令格式严格**：所有命令必须以`CMD:`开头
2. **参数分隔符**：使用冒号`:`分隔参数
3. **类型范围**：类型ID必须在1-3之间
4. **索引有效性**：删除操作需要确保索引存在
5. **路径长度限制**：路径字符串受到缓冲区大小限制
6. **权限要求**：此处使用echo写入命令需要root
