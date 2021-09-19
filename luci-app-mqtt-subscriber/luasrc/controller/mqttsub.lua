module("luci.controller.mqttsub", package.seeall)

function index()
    entry(
        {"admin", "services", "mqttsub"},
        firstchild(), "MQTT Subscriber", 151
    )
    entry(
        {"admin", "services", "mqttsub", "settings"},
        cbi("settings"), "Settings", 1
    )
    entry(
        {"admin", "services", "mqttsub", "messages"},
        form("messages"), "Messages", 2
    )

    entry(
        {"admin", "services", "mqttsub", "get_messages"},
        post("get_messages"), nil
    ).leaf = true

    entry(
        {"admin", "services", "mqttsub", "events"},
        arcombine(
            cbi("events/events"),
            cbi("events/event_details")
        ),
        _("Events list"),
        3
    ).leaf=true
end

function get_messages()
    local msgs = {}
    local sql = require "lsqlite3"
    local db = sql.open("/var/lib/mqttsub.db")
    
    db:exec([[
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY,
            date DATETIME DEFAULT current_timestamp,
            topic TEXT NOT NULL,
            message TEXT NOT NULL
        );
    ]])

    if db then
        for r in db:rows("SELECT id, date, topic, message FROM messages ORDER BY date DESC") do
            -- Keys to be inserted must match names from model!
            table.insert(msgs, {id = r[1], date = r[2], topic = r[3], message = r[4]})
        end
        db:close()
    end

    luci.http.prepare_content("application/json")
    luci.http.write_json(msgs)
end
