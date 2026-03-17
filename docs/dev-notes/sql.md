# sql 命令

* 查询条目限制

```sql
# MySQL / MariaDB / PostgreSQL / SQLite
# 对查询结果排序时需要子语句

# 查询 user 表中第10-19条数据
SELECT account_id, nickname FROM user LIMIT 10 OFFSET 9;

# 排序后查询 user 表中第10-19条数据
SELECT account_id, nickname FROM user 
ORDER BY id ASC
LIMIT 11 OFFSET 9;

# 查询 user 表中第10-19条数据的account_id和nickname字段，并对查询到的数据以nickname正序排序
SELECT account_id, nickname 
FROM (
    # 需要使用子查询语句
    SELECT account_id, nickname 
    FROM user 
    LIMIT 10 OFFSET 9
) AS sub
ORDER BY nickname ASC;

# 查询 user 表中以 register_time 正序排序的 nickname 为 'nickname' 的 10 - 20 条的 account_id 和 nickname 字段
# 子句的顺序是固定的，正确顺序：SELECT ... WHERE ... ORDER BY ... LIMIT ... OFFSET ...
CREATE INDEX idx_user_query ON user(nickname, register_time); # 可以先创建索引
SELECT account_id, nickname # 可以不输出 register_time
FROM user 
WHERE nickname = 'nickname' 
ORDER BY register_time ASC 
LIMIT 10 OFFSET 9;
```

* 使用索引

```sql
# 1. 创建索引后会自动调用，不需要修改查询命令
# 2. 增删改时不需要手动刷新索引

# 创建索引
CREATE INDEX idx_user_nickname ON user(nickname);

# 使用
-- 此时 SQLite 会自动使用 idx_user_nickname 快速定位，而不是一行行找
SELECT account_id, nickname FROM user WHERE nickname = '张三';
-- 排序时也会自动使用索引提高速度
SELECT * FROM user ORDER BY nickname ASC;
```

* 查询存在

```sql
# `SELECT 1` 中的 `1` 代表返回的结果，如果查到了那么就返回 `1`
# `LIMIT 1` 可以明确告诉数据库引擎：“只要找到第一个匹配的就立刻停止扫描”
SELECT 1 FROM users WHERE account_id = '1001' LIMIT 1;

# // 在代码中如何使用（以 C++ 为例） // 伪代码
# string sql = "SELECT 1 FROM users WHERE account_id = '1001' LIMIT 1;";
# auto result = db.execute(sql);
# if (!result.empty()) {
#     // 用户存在
# }

```

* 参数绑定：

```c++
// * `SELECT 1 FROM users WHERE account_id = '{}';` :
//   使用了 `{}`,直接替换可能会发生数据库注入，应该使用 参数化查询
//   * 错误（危险）：`format("... WHERE id = '{}'", userInput)`
//   * 正确（安全）：`SELECT 1 FROM users WHERE account_id = ?` 然后绑定参数。
//
// * sqlite 的参数绑定：
//   * 使用 ? 作为匿名占位符
//     `SELECT 1 FROM user WHERE account_id = ? LIMIT 1;`
//   * 使用命名占位符（更清晰）
//     `SELECT 1 FROM user WHERE account_id = :id LIMIT 1;`
// * 常见的绑定函数
//   根据字段类型选择对应的函数：
//   * 整数      : `sqlite3_bind_int(stmt, index, value)`
//   * 64 位整数 : `sqlite3_bind_int64(stmt, index, value)`
//   * 浮点数    : `sqlite3_bind_double(stmt, index, value)`
//   * 字符串    : `sqlite3_bind_text(stmt, index, value, length, destructor)`
//   * 空值      : `sqlite3_bind_null(stmt, index)`

// C++ 常用写法（使用原生 SQLite3 库）
// 需要遵循 Prepare -> Bind -> Step -> Finalize 这个流程：
sqlite3_stmt* stmt;
const char* sql = "SELECT 1 FROM user WHERE account_id = ? LIMIT 1;";
// 1. 编译 SQL 语句
sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
// 2. 绑定参数 (注意：索引从 1 开始)
// 假设 account_id 是字符串，使用 text 绑定
sqlite3_bind_text(stmt, 1, "my_account_id", -1, SQLITE_STATIC);
// 3. 执行并获取结果
int rc = sqlite3_step(stmt);
if (rc == SQLITE_ROW) {
    // 找到了记录！存在该用户
} else if (rc == SQLITE_DONE) {
    // 没找到记录
}
// 4. 清理
sqlite3_finalize(stmt);
```
