local m, s, o

m = SimpleForm("system")
m.submit = false
m.reset = false

local s = m:section(Table, nil, translate("Received MQTT Messages"), translate("Messages received from MQTT publisher(s)"))
s.anonymous = true
s.template = "mqtt-subscriber/messages"
s.addremove = true
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

o = s:option(DummyValue, "id", translate("ID"), translate("Message ID from database"))
o.optional = true
o.id = true

s:option(DummyValue, "date", translate("Date"), translate("Received message creation date"))
s:option(DummyValue, "topic", translate("Topic"), translate("Message topic"))

o = s:option(DummyValue, "message", translate("Message Body"), translate("Content of the message"))
o.message = true

function s.remove(self, id)
    if id then
        local sql = require "lsqlite3"
        local db = sql.open("/var/lib/mqttsub.db")

        if db then
            db:exec("DELETE FROM messages WHERE id = " .. id .. ";")
            db:close()
            luci.util.perror("DEBUG: Removed message with id = " .. id)
        end
    end
end

return m
