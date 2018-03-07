
m = Map("powquty", "PowQuty Configuration")
s = m:section(TypedSection, "powquty", "powquty", "Configuration")

s:tab("general", "General Configuration", translate(""))
s:tab("mqtt", "MQTT Configuration", translate(""))
s:tab("slack", "Slack Configuration", translate(""))

-- general
device_tty = s:taboption("general", Value, "device_tty", "Device tty", "The path to the tty device created by cdc-acm driver")
device_tty.datatype = "string"
device_tty.default = "/dev/ttyACM0"

dev_uuid = s:taboption("general", Value, "dev_uuid", "dev_uuid", "The dev_uuid sets the device name used in the MQTT-publish messages")
dev_uuid.datatype = "string"
dev_uuid.default = "BERTUB001"

powquty_path = s:taboption("general", Value, "powquty_path", "powquty_path", "The path of the output logfile.")
powquty_path.datatype = "string"
powquty_path.default = "/tmp/powquty.log"

powquty_event_path = s:taboption("general", Value, "powquty_event_path", "powquty_event_path", "The path of the event output logfile")
powquty_event_path = "string"
powquty_event_path = "/tmp/powqutyy_event.log"

powqutyd_print = s:taboption("general", Flag, "powqutyd_print", "powqutyd_print", "If activated, will print the results published to the MQTT broker to stdout")
powqutyd_print.rmempty = false
powqutyd_print.default = true

max_log_size_kb = s:taboption("general", Value, "max_log_size_kb", "max_log_size_kb", "Maximum size of log files in kB")
max_log_size_kb.datatype = "string"
max_log_size_kb.default = "4096"

dev_lat = s:taboption("general", Value, "dev_lat", "dev_lat", "Device Latitude")
dev_lat.datatype = "string"
dev_lat.default = "55.0083525"

dev_lon = s:taboption("general", Value, "dev_lon", "dev_lon", "Device Longitude")
dev_lon.datatype = "string"
dev_lon.default = "82.935732"

dev_acc = s:taboption("general", Value, "dev_acc", "dev_acc", "GPS accuracy")
dev_acc.datatype = "string"
dev_acc.default = "0"

dev_alt = s:taboption("general", Value, "dev_alt", "dev_alt", "Device altitude")
dev_alt.datatype = "string"
dev_alt.default = "0"

-- mqtt
use_metadata = s:taboption("mqtt", Flag, "use_metadata", "use_metadata", "Send metadata with MQTT")
use_metadata.rmempty = false
use_metadata.default = false

meta_comment = s:taboption("mqtt", Value, "meta_comment", "meta_comment", "Set comment for node")
meta_comment.datatype = "string"
meta_comment.default = ""

meta_id = s:taboption("mqtt", Value, "meta_id", "meta_id", "Set node id")
meta_id.datatype = "string"
meta_id.default = ""

meta_operator = s:taboption("mqtt", Value, "meta_operator", "meta_operator", "Set node operator")
meta_operator.datatype = "string"
meta_operator.default = ""

meta_phase = s:taboption("mqtt", Value, "meta_phase", "meta_phase", "Set measurement phase")
meta_phase.datatype = "string"
meta_phase.default = ""

meta_reason = s:taboption("mqtt", Value, "meta_reason", "meta_reason", "Set a reason")
meta_reason.datatype = "string"
meta_reason.default = ""

meta_type = s:taboption("mqtt", Value, "meta_type", "meta_type", "Set a measurement type")
meta_type.datatype = "string"
meta_type.default = ""

mqtt_uname = s:taboption("mqtt", Value, "mqtt_uname", "mqtt_uname", "Username for MQTT server")
mqtt_uname.datatype = "string"
mqtt_uname.default = "username"

mqtt_pw = s:taboption("mqtt", Value, "mqtt_pw", "mqtt_pw", "Password for MQTT Server")
mqtt_pw.datatype = "string"
mqtt_pw.default = "password"

mqtt_host = s:taboption("mqtt", Value, "mqtt_host", "mqtt_host", "IP-address or URL to the MQTT broker who receives the publish messages of powqutd")
mqtt_host.datatype = "string"
mqtt_host.default = "localhost"

mqtt_topic = s:taboption("mqtt", Value, "mqtt_topic", "mqtt_topic", "The topic under which powquty will publish the mesurement results. The Format is the following: device_uuid,timestamp(sec),timestamp(millisec),3,RMS_Voltage,RMS_Frequency,H3,H5,H7,H9,H11,H13,H15")
mqtt_topic.datatype = "string"
mqtt_topic.default = "devices/update"

-- slack
slack_webhook = s:taboption("slack", Value, "slack_webhook", "slack_webhook", "Set webhook to use with slack")
slack_webhook.datatype = "string"
slack_webhook.default = ""

slack_channel = s:taboption("slack", Value, "slack_channel", "slack_channel", "Set Slack channel")
slack_channel.datatype = "string"
slack_channel.default = "#general"

slack_user = s:taboption("slack", Value, "slack_user", "slack_user", "Set Slack user")
slack_user.datatype = "string"
slack_user.default = "PowQutyEvent"

slack_notification = s:taboption("slack", Flag, "slack_notification", "slack_notification", "Enable to send slack notifications about EN50160 event")
slack_notification.rmempty = false
slack_notification.default = false

return m

