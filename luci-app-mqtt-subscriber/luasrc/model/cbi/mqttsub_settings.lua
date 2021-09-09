local certs = require "luci.model.certificate"
local certificates = certs.get_certificates()
local keys = certs.get_keys()
local cas = certs.get_ca_files().certs

m = Map("mqttsub")

s = m:section(NamedSection, "mqttsub_settings", "mqttsub", "MQTT Subscriber General Settings")

-- Base settings
enabled = s:option(Flag, "enabled", translate("Enable"), translate("Select to enable MQTT subscriber"))

host = s:option(Value, "host", translate("Hostname"), translate("Specify address of the subscriber"))
host.placeholder  = "localhost"
host:depends("enabled", "1")
host.datatype = "host"
host.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqttsub.mqttsub_settings.enabled")
	local value = self:formvalue(section)
	if enabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: hostname is empty")
	end
	Value.parse(self, section, novld, ...)
end

port = s:option(Value, "port", translate("Port"), translate("Specify port of the subscriber"))
port.default = "1883"
port.placeholder = "1883"
port:depends("enabled", "1")
port.datatype = "port"
port.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqttsub.mqttsub_settings.enabled")
	local value = self:formvalue(section)
	if enabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: port is empty")
	end
	Value.parse(self, section, novld, ...)
end

username = s:option(Value, "username", translate("Username"), translate("Specify username (optional)"))
username.datatype = "credentials_validate"
username.placeholder = translate("Username")
username:depends("enabled", "1")

password = s:option(Value, "password", translate("Password"), translate("Specify password (optional)"))
password:depends("enabled", "1")
password.password = true
password.datatype = "credentials_validate"
password.placeholder = translate("Password")


-- Security settings
FileUpload.unsafeupload = true

use_tls_ssl = s:option(Flag, "use_tls_ssl", translate("Use TLS/SSL"), translate("Mark to use TLS/SSL for connection"))
use_tls_ssl.rmempty = false

tls_type = s:option(ListValue, "tls_type", translate("TLS Type"), translate("Select the type of TLS encryption"))
tls_type:depends("use_tls_ssl", "1")
tls_type:value("cert", translate("Certificate based"))
tls_type:value("psk", translate("Pre-Shared-Key based"))

local certificates_link = luci.dispatcher.build_url("admin", "system", "admin", "certificates")
o = s:option(Flag, "_device_sec_files", translate("Certificate files from device"),
	translatef("Choose this option if you want to select certificate files from device.\
	Certificate files can be generated <a class=link href=%s>%s</a>", certificates_link, translate("here")))
o:depends({use_tls_ssl="1", tls_type = "cert"})

ca_file = s:option(FileUpload, "ca_file", translate("CA File"), translate("Upload CA file"));
ca_file:depends({use_tls_ssl="1", _device_sec_files="", tls_type = "cert"})

cert_file = s:option(FileUpload, "cert_file", translate("CERT File"), translate("Upload CERT file"));
cert_file:depends({use_tls_ssl="1", _device_sec_files="", tls_type = "cert"})

key_file = s:option(FileUpload, "key_file", translate("Key File"), translate("Upload Key file"));
key_file:depends({use_tls_ssl="1", _device_sec_files="", tls_type = "cert"})

ca_file = s:option(ListValue, "_device_ca_file", translate("CA File"), translate("Upload CA file"));
ca_file:depends({_device_sec_files = "1", tls_type = "cert"})

if #cas > 0 then
	for _,ca in pairs(cas) do
		ca_file:value("/etc/certificates/" .. ca.name, ca.name)
	end
else 
	ca_file:value("", translate("-- No file available --"))
end

function ca_file.write(self, section, value)
	m.uci:set(self.config, section, "ca_file", value)
end

ca_file.cfgvalue = function(self, section)
	return m.uci:get(m.config, section, "ca_file") or ""
end

cert_file = s:option(ListValue, "_device_cert_file", translate("CERT File"), translate("Upload CERT file"));
cert_file:depends({_device_sec_files = "1", tls_type = "cert"})

if #certificates > 0 then
	for _,cert in pairs(certificates) do
		cert_file:value("/etc/certificates/" .. cert.name, cert.name)
	end
else 
	cert_file:value("", translate("-- No file available --"))
end

function cert_file.write(self, section, value)
	m.uci:set(self.config, section, "cert_file", value)
end

cert_file.cfgvalue = function(self, section)
	return m.uci:get(m.config, section, "cert_file") or ""
end

key_file = s:option(ListValue, "_device_key_file", translate("Key File"), translate("Upload Key file"));
key_file:depends({_device_sec_files = "1", tls_type = "cert"})

if #keys > 0 then
	for _,key in pairs(keys) do
		key_file:value("/etc/certificates/" .. key.name, key.name)
	end
else 
	key_file:value("", translate("-- No file available --"))
end

function key_file.write(self, section, value)
	m.uci:set(self.config, section, "key_file", value)
end

key_file.cfgvalue = function(self, section)
	return m.uci:get(m.config, section, "key_file") or ""
end

tls_version = s:option(ListValue, "tls_version", translate("TLS version"), translate("Used TLS version"));
tls_version:depends({tls_type = "cert"})
tls_version:value("tlsv1", "tlsv1");
tls_version:value("tlsv1.1", "tlsv1.1");
tls_version:value("tlsv1.2", "tlsv1.2");
tls_version:value("all", "Support all");
tls_version.default = "all"

o = s:option(Value, "psk", translate("Pre-Shared-Key"), translate("The pre-shared-key in hex format with no leading “0x”"))
o.datatype = "lengthvalidation(0, 128)"
o.placeholder = "Key"
o:depends({use_tls_ssl = "1", tls_type = "psk"})

o = s:option(Value, "identity", translate("Identity"), translate("Specify the Identity"))
o.datatype = "uciname"
o.placeholder = "Identity"
o:depends({use_tls_ssl = "1", tls_type = "psk"})


-- Subscriber topics
st = m:section(TypedSection, "topic", translate("Topics"), translate(""))
st.addremove = true
st.anonymous = true
st.template = "mqtt-subscriber/topicsmenu"
st.novaluetext = translate("There are no topics created yet.")

topic = st:option(Value, "topic", translate("Topic name"), translate(""))
topic.datatype = "string"
topic.maxlength = 65536
topic.placeholder = translate("Topic")
topic.rmempty = false
topic.parse = function(self, section, novld, ...)
	local value = self:formvalue(section)
	if value == nil or value == "" then
		self.map:error_msg(translate("Topic name can not be empty"))
		self.map.save = false
	end
	Value.parse(self, section, novld, ...)
end

qos = st:option(
    ListValue, "qos", translate("QoS level"),
    translate("The QoS level used for this topic")
)
qos:value("0", "At most once (0)")
qos:value("1", "At least once (1)")
qos:value("2", "Exactly once (2)")
qos.rmempty=false
qos.default="0"

return m
