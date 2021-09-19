local ds = require "luci.dispatcher"

m = Map("mqttsub", translate(""))

s = m:section(
	TypedSection,
	"event",
	translate("MQTT Subscriber Events Reporting Rules"),
	translate("This section displays MQTT subscriber events reporting rules.")
)
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sortable  = true
s.extedit   = ds.build_url("admin/services/mqttsub/events/%s")
s.template_addremove = "cbi/add_rule"
s.novaluetext = translate("There are no events reporting rules created yet")
s.delete_alert = true
s.alert_message = translate("Are you sure you want to delete this rule?")

function s.create(self, section)
	created = TypedSection.create(self, section)
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)
	if created then
		m.uci:save(self.config)
		luci.http.redirect(ds.build_url("admin/services/mqttsub/events", created))
	else
		m.uci:save(self.config)
		m.uci.commit(self.config)
	end
end

topic = s:option(
	DummyValue,
	"topic",
	translate("Topic Name"),
	translate("MQTT subscribed topic to observe")
)

key = s:option(
	DummyValue,
	"key",
	translate("Key Name"),
	translate("Key name of the received JSON to compare")
)

vt = s:option(
	DummyValue,
	"type",
	translate("Value Type"),
	translate("Event type for which the rule is applied")
)
vt.width = "20%"

function vt.cfgvalue(self, s)
	local vt = self.map:get(s, "type")
	if vt == "1" then
		return "String"
	elseif vt == "2" then
		return "Decimal"
	else
	    return "Null"
	end
end

ct = s:option(
	DummyValue,
	"ct",
	translate("Comparison Type"),
	translate("Compares set value with recieved")
)
ct.width = "10%"

function ct.cfgvalue(self, s)
	local ct = self.map:get(s, "ct")
	if ct == "0" then
		return "=="
	elseif ct == "1" then
		return "!="
	elseif ct == "2" then
		return ">"
	elseif ct == "3" then
		return ">="
	elseif ct == "4" then
		return "<"
	else
	    return "<="
	end
end

value = s:option(
	DummyValue,
	"value",
	translate("Value"),
	translate("Value to compare with")
)

en = s:option(Flag, "enabled", translate(""))
en.width = "15%"
en.default = en.enabled
en.rmempty = false
en.last = true

return m
