local dsp = require "luci.dispatcher"

local m, s, o
arg[1] = arg[1] or ""

m = Map(
	"mqttsub",
	translate("MQTT Subscriber Events Reporting Configuration"),
	translate("This section is used to customize how an Events Reporting rule will function.")
)

m.redirect = dsp.build_url("admin/services/mqttsub/events")
if m.uci:get("mqttsub", arg[1]) ~= "event" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, arg[1], "event", translate("MQTT Event"))
s.anonymous = true
s.addremove = false

o = s:option(
	Flag,
	"enabled",
	translate("Enable"),
	translate("Turns the event on or off.")
)
o.rmempty = false

local is_topic = false
topic = s:option(ListValue, "topic", translate("Topic"), translate(""))
m.uci:foreach("mqttsub", "topic", function(s)
	if s.topic then
		topic:value(s.topic, s.topic)
		is_topic = true
	end
end)
if not is_topic then
	topic:value(0, translate("No topics created"))
end

key = s:option(
	Value,
	"key",
	translate("Key"),
	translate([[
		JSON key of a topic payload. 
		Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )
	]])
)
key.datatype = "fieldvalidation('^[a-zA-Z0-9!@#%$%%&%*%+%-/=%?%^_`{|}~%. ]+$',0)"

val = s:option(
	Value,
	"value",
	translate("Value"),
	translate("")
)
val.rmempty = false
val.parse = function(self, section, novld, ...)
	local value = self:formvalue(section)
	if value == nil or value == "" then
		self.map:error_msg(translate("Value cannot be empty"))
		self.map.save = false
	end
	Value.parse(self, section, novld, ...)
end

vt = s:option(
	ListValue,
	"type",
	translate("Value type"),
	translate("")
)
vt:value(0, "Null")
vt:value(1, "String")
vt:value(2, "Decimal")

ct = s:option(
	ListValue,
	"ct",
	translate("Comparison type"),
	translate("Compares set value with received value")
)
ct:value(0, "==")
ct:value(1, "!=")
ct:value(2, ">")
ct:value(3, ">=")
ct:value(4, "<")
ct:value(5, "<=")

local is_group = false
mailGroup = s:option(
	ListValue,
	"emailgroup",
	translate("Email account"),
	translate([[
		Recipient's email configuration <br/>(<a href="/cgi-bin/luci/admin
		/system/admin/group/email" class="link">configure it here</a>)
	]])
)
m.uci:foreach("user_groups", "email", function(s)
	if s.senderemail then
		mailGroup:value(s.name, s.name)
		is_group = true
	end
end)
if not is_group then
	mailGroup:value(0, translate("No email accounts created"))
end

function mailGroup.parse(self, section, novld, ...)
	local val = self:formvalue(section)
	if val and val == "0" then
		self:add_error(section, "invalid", translate("No email accounts selected"))
	end
	Value.parse(self, section, novld, ...)
end

recipient = s:option(
	Value,
	"to_email",
	translate("Recipient's email address"),
	translate([[
		For whom you want to send an email to.
		Allowed characters (a-zA-Z0-9._%+@-)
	]])
)
recipient.datatype = "email"
recipient.placeholder = "mail@domain.com"
recipient.parse = function(self, section, novld, ...)
	local value = self:formvalue(section)
	if value == nil or value == "" then
		self.map:error_msg(translate("Recipient cannot be empty"))
		self.map.save = false
	end
	Value.parse(self, section, novld, ...)
end


function m.on_save(self)
	local group_name = m:formvalue("cbid.mqttsub."..arg[1]..".emailgroup")
	local group
	m.uci:foreach("user_groups", "email", function(s)
		if s.name == group_name then
			group = s[".name"]
		end
	end)
	local smtpIP = m.uci:get("user_groups", group, "smtp_ip") 
	local smtpPort = m.uci:get("user_groups", group, "smtp_port") 
	local username = m.uci:get("user_groups", group, "username") 
	local passwd = m.uci:get("user_groups", group, "password") 
	local senderEmail = m.uci:get("user_groups", group, "senderemail") 
	local secure = m.uci:get("user_groups", group, "secure_conn") 
	m.uci:delete("mqttsub", arg[1], "smtp_username")
	m.uci:delete("mqttsub", arg[1], "smtp_password")
	m.uci:set("mqttsub", arg[1], "from_email", senderEmail)
	m.uci:set("mqttsub", arg[1], "smtp_password", passwd)
	m.uci:set("mqttsub", arg[1], "smtp_username", username)
	m.uci:set("mqttsub", arg[1], "smtp_port", smtpPort)
	m.uci:set("mqttsub", arg[1], "smtp_host", smtpIP)
	m.uci:set("mqttsub", arg[1], "smtp_use_ssl", secure)
end

return m
