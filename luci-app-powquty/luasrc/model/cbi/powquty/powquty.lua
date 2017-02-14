
m = Map("powquty")

s = m:section(NamedSection, "powquty", "powquty", "Configuration")


device_tty = s:option(Value, "device_tty", "Device tty", "The path to the tty device created by cdc-acm driver")
device_tty.datatype = "string"
device_tty.default = "/dev/ttyACM0"

mqtt_host = s:option(Value, "mqtt_host", "mqtt_host", "IP-address or URL to the MQTT broker who receives the publish messages of powqutd")
mqtt_host.datatype = "string"
mqtt_host.default = "localhost"

mqtt_topic = s:option(Value, "mqtt_topic", "mqtt_topic", "The topic under which powquty will publish the mesurement results. The Format is the following: device_uuid,timestamp(sec),timestamp(millisec),3,RMS_Voltage,RMS_Frequency,H3,H5,H7,H9,H11,H13,H15")
mqtt_topic.datatype = "string"
mqtt_topic.default = "devices/update"

dev_uuid = s:option(Value, "dev_uuid", "dev_uuid", "The dev_uuid sets the device name used in the MQTT-publish messages")
dev_uuid.datatype = "string"
dev_uuid.default = "BERTUB001"

powquty_path = s:option(Value, "powquty_path", "powquty_path", "The path of the output logfile.")
powquty_path.datatype = "string"
powquty_path.default = "/tmp/powquty.log"

powqutyd_print = s:option(Flag, "powqutyd_print", "powqutyd_print", "If activated, will print the results published to the MQTT broker to stdout")
powqutyd_print.rmempty = false
powqutyd_print.default = true

max_log_size_kb = s:option(Value, "max_log_size_kb", "max_log_size_kb", "Maximum size of log files in kB")
max_log_size_kb.datatype = "string"
max_log_size_kb.default = "4096"

return m

