
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

powqutyd_print = s:taboption("general", Flag, "powqutyd_print", "powqutyd_print", "If activated, will print the results published to the MQTT broker to stdout")
powqutyd_print.rmempty = false
powqutyd_print.default = true

max_log_size_kb = s:taboption("general", Value, "max_log_size_kb", "max_log_size_kb", "Maximum size of log files in kB")
max_log_size_kb.datatype = "string"
max_log_size_kb.default = "4096"

-- mqtt
mqtt_host = s:taboption("mqtt", Value, "mqtt_host", "mqtt_host", "IP-address or URL to the MQTT broker who receives the publish messages of powqutd")
mqtt_host.datatype = "string"
mqtt_host.default = "localhost"

mqtt_topic = s:taboption("mqtt", Value, "mqtt_topic", "mqtt_topic", "The topic under which powquty will publish the mesurement results. The Format is the following: device_uuid,timestamp(sec),timestamp(millisec),3,RMS_Voltage,RMS_Frequency,H3,H5,H7,H9,H11,H13,H15")
mqtt_topic.datatype = "string"
mqtt_topic.default = "devices/update"

mqtt_event_host = s:taboption("mqtt", Value, "mqtt_event_host", "mqtt_event_host", "IP-address or URL to the MQTT broker who receives the publish message of powquty events")
mqtt_event_host.datatype = "string"
mqtt_event_host.default = "localhost"

mqtt_event_topic = s:taboption("mqtt", Value, "mqtt_event_topic", "mqtt_event_topic", "The topic under which powquty will publish EN50160 events")
mqtt_event_topic.datatype = "string"
mqtt_event_topic.default = "device/en50160-event"

mqtt_event_flag = s:taboption("mqtt", Flag, "mqtt_event_flag", "mqtt_event_flag", "If activated powquty will send notifications via mqtt about EN50160 events")
mqtt_event_flag.rmempty = false
mqtt_event_flag.default = false

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

