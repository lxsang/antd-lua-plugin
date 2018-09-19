auth_or_die("User unauthorized. Please login")
local user = require("system.uman").userinfo(SESSION.iotos_user)
result(user)
