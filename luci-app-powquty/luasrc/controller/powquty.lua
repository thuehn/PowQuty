module("luci.controller.powquty", package.seeall)

function index()
   if not nixio.fs.access("/etc/config/powquty") then
      return
   end
   entry({"admin", "services", "powquty"}, cbi("powquty/powquty"), _("Powquty"))
end

