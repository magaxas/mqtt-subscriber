module("luci.controller.mqttsub", package.seeall)

function index()
    entry(
        {"admin", "services", "mqttsub"},
        firstchild(), "MQTT Subscriber", 151
    )
    entry(
        {"admin", "services", "mqttsub", "settings"},
        cbi("mqttsub_settings"), "Settings", 1
    )
    entry(
        {"admin", "services", "mqttsub", "messages"},
        cbi("mqttsub_messages"), "Messages", 2
    )
end

function get_messages()
    local msgs = {}
    local sql = require "lsqlite3"
    local db = sql.open("/var/lib/mqttsub.sql")
    
    db:exec([[
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY,
            date DATETIME DEFAULT current_timestamp,
            topic TEXT NOT NULL,
            message TEXT NOT NULL
        );
    ]])

    if db then
        for r in db:rows("SELECT * FROM messages ORDER BY date DESC") do
            -- Keys to be inserted must match names from model!
            table.insert(msgs, {date = r[1], topic = r[2], message = r[3]})
        end
        db:close()
    end

    return msgs
end
