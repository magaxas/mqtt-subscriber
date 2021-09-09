local m, s, o

m = SimpleForm("system")
m.submit = false
m.reset = false

local s = m:section(Table, messages, translate("Received MQTT Messages"), translate("Messages received from MQTT publisher(s)"))
s.anonymous = true
s.template = "mqtt-subscriber/messages"
s.addremove = false
s.refresh = true
s.table_config = {
    truncatePager = true,
    labels = {
        placeholder = "Search...",
        perPage = "Messages per page {select}",
        noRows = "No messages found",
        info = ""
    },
    layout = {
        top = "<table><tr><td>{select}</td><td>{search}</td></tr></table>",
        bottom = "{info}{pager}"
    }
}

o = s:option(DummyValue, "date", translate("Date"), translate("Received message creation date"))
o = s:option(DummyValue, "topic", translate("Topic"), translate("Message topic"))
o = s:option(DummyValue, "message", translate("Message Body"), translate("Content of the message"))

if s.addremove then
    s:option(DummyValue, "", translate(""))
end

return m
