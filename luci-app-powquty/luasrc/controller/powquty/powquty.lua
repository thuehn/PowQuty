module("luci.controller.powquty.powquty", package.seeall)


require ("lfs")
-- require("luci.i18n")

-- Routing and menu index for the luci dispatcher.
function index()

 	if not nixio.fs.access("/etc/config/powquty") then
      return
   	end
   	entry({"admin", "services", "powquty"}, cbi("powquty/powquty"), _("Powquty"))
   	
    -- entry for menu node
    entry ( { "admin", "statistics", "powquty" }, firstchild (), "powquty", 60 ).dependent=false
    
    -- entry and route for the graph page
    entry ( { "admin", "statistics", "powquty", "graph" }, template ( "powquty/graph" ), "Graph", 1)  

    -- route of the image
    local vars = luci.http.formvalue(nil, true)
	local span = vars.timespan or nil
    local phys = vars.phys or "1"
    local img = vars.img or nil
	entry ( { "admin", "statistics", "powquty", "graph" }, 
               call ( "powquty_render" ), "Graph", 3 ).query = { timespan = span
                                                               , phys = phys 
                                                               , img = img
                                                               }
end


-- Returns a rrd DEF declaration for a metric
function rrd_metric_def ( phy, metric, rrd_path, file_prefix, rrd_suffix, column_name )
    local cmd = " \"DEF:" .. metric .. "0" .. "=" 
              .. rrd_path .. "/" .. file_prefix .. "-" .. metric .. "0" .. rrd_suffix
              .. ":" .. column_name ..":AVERAGE\" \\\n"
    return cmd
end


-- Returns a legend declaration for a metric
function rrd_metric_legend ( metric ) 
    --local var = "rel_" .. metric
    local var = metric
    return " \"GPRINT:" .. var .. ":MIN:\t\tMin\\: %8.2lf%s \" \\\n"
             .. " \"GPRINT:" .. var .. ":AVERAGE:\tAvg\\: %8.2lf%s \" \\\n"
             .. " \"GPRINT:" .. var .. ":MAX:\tMax\\: %8.2lf%s \\n\" \\\n"
end


-- Returns rrd command line parts for a metric.
--
-- phy: zero based phy number
-- metric: name of the metric
-- rrd_path: path to rrd databases
-- file_prefix: prefix of the rrd file (mostly the type, i.e. gauge, derive)
-- rrd_suffic: rrd database file extension with a leading dot
-- column_name: name of the rrd column
-- stacked: whether the shape should be declared stacked
-- color: main color of the shape
function rrd_metric_defs ( phy, metric, rrd_path, file_prefix, rrd_suffix, column_name )

    -- declare metric
    local cmd = rrd_metric_def ( phy, metric, rrd_path, file_prefix, rrd_suffix, column_name)

--    -- absolute values, use with 'GAUGE' rrd
    --cmd = cmd .. " \"CDEF:rel_" .. metric .. "=" .. "voltage" .. "," .. metric .. ",/\" \\\n"

--    -- percentage output, use with 'DERIVE' rrd
--    cmd = cmd .. " \"CDEF:rel_" .. metric .. phy .. "=" .. metric .. phy .. ",100,*,abs_count" .. phy .. ",/\" \\\n"

    return cmd
end


function rrd_metric_shape ( phy, metric, stacked, shape, color )
    local var = metric .. "0"
    local cmd = " " .. shape .. ":"
        .. var
        .. color .. ":" .. metric .. " \\\n"
    return cmd
end


-- Creates rrd shell command and executes it.
--
-- phy: zero based number of the mac phy
-- image: path of the resulting image file
-- span: timespan as 1hour, 1day, ...
-- width: resulting image width
-- height: resulting image height
-- rrd_path: path to rrd databases
-- metrics: names of all metrics. rrd files should match
--          pattern "gauge-[metric].rrd" or derive-[metric].rrd like collectd does.
-- shape: shape of the graph (LINE2 or AREA)
-- stacked: whether the shapes should be declared stacked
function generate_rrdimage ( phy, image, span, width, height, rrd_path,
                             metrics, shape, stacked, highlight, busy_metric ) 

    local rrd_suffix = ".rrd"
    local column_name = "value"
    local file_prefix = "gauge"
--  local file_prefix = "derive"


    local span_seconds = luci.util.parse_units( span )

    local cmd = "rrdtool graph "
    cmd = cmd .. image 

    cmd = cmd .. " --end now" .. " --start end-" .. span_seconds .. "s"
    
    local upper_limit = 240
    local lower_limit = 220
    local vertical_label = "RMS(Voltage) [V]"
    
    if (phy == 0) then
    elseif (phy == 1) then
        upper_limit = 51
        lower_limit = 49
        vertical_label = "Frequency [Hz]"
    elseif (phy == 2) then
      	upper_limit = 16
      	lower_limit = 0
      	vertical_label = "Harmonics [V]"
    end
    
    cmd = cmd .. " --upper-limit " .. upper_limit .. " --lower-limit " .. lower_limit .. " --alt-autoscale-max"
    
    
    cmd = cmd .. " --vertical-label \"" .. vertical_label .. "\""
    cmd = cmd .. " --width " .. width
    cmd = cmd .. " --height " .. height .. " \\\n"

    -- add a dense grid when timespan becomes small
    if (span_seconds <= 60) then
        cmd = cmd .. " --x-grid SECOND:2:MINUTE:1:SECOND:10:0:\%X"
    elseif (span_seconds <= 300) then
        cmd = cmd .. " --x-grid SECOND:10:MINUTE:1:SECOND:30:0:\%X"
    elseif (span_seconds <= 1800) then
        cmd = cmd .. " --x-grid MINUTE:5:SECOND:30:MINUTE:5:0:\%X"
    end

    local colors = { "#FF5555", "#55FF55", "#5555FF", "#FF55FF", "#55FFFF", "#FFFF55" }
    local colors2 = { "#AA0000", "#00AA00", "#0000AA", "#AA00AA", "#00AAAA", "#AAAA00" }

    -- fixme: redeclaration of "abs_count" metric name (first in /usr/bin/regmon-genconfig)
    --cmd = cmd .. rrd_metric_def ( phy, vertical_label, rrd_path, file_prefix, rrd_suffix, column_name )

    -- print defs for each metric
    for i, metric in ipairs ( metrics ) do
        cmd = cmd .. rrd_metric_defs ( phy, metric, rrd_path, file_prefix, rrd_suffix, column_name )
    end

    -- add def for other metrics than the explicit monitored one
    -- i.e. "noise = busy - rx - tx"
    --cmd = cmd .. " \"CDEF:rel_noise" .. phy .. "=rel_" .. busy_metric .. phy
   -- for i, metric in ipairs ( metrics ) do
   --     if ( metric ~= busy_metric ) then
   --         cmd = cmd .. ",rel_" .. metric .. phy.. ",-"
   --     end
   -- end
    --cmd = cmd .. "\" \\\n"

    -- normalize noise
    --cmd = cmd .. " \"CDEF:rel_noise" .. phy .. "_norm=rel_noise" .. phy .. ",0,LT,0,rel_noise" .. phy .. ",IF\" \\\n"

    -- calculate idle = (abs_count - busy_count) * 100 / abs_count
    --cmd = cmd .. " \"CDEF:rel_idle" .. phy .. "=abs_count" .. phy .. "," 
     --       .. busy_metric .. phy .. ",-,100,*,abs_count" .. phy .. ",/\" \\\n"

    -- normalize idle
    --cmd = cmd .. " \"CDEF:rel_idle" .. phy .. "_norm=rel_noise" .. phy 
    --    .. ",0,LT,rel_idle" .. phy .. ",rel_noise" .. phy .. ",+,rel_idle" .. phy .. ",IF\" \\\n"

    
    --cmd = cmd .. " COMMENT:\"relative mac states\\n\" \\\n"
    local out_shape = shape
    -- print shapes and legends for each metric
    for i, metric in ipairs ( metrics ) do
        if ( metric ~= busy_metric ) then
            if ( stacked == '1' and i > 1 ) then
                out_shape = "STACK"
            end
            cmd = cmd .. rrd_metric_shape ( phy, metric, stacked, out_shape, colors[i] )
            if (phy == 0 or phy == 1) then
            	cmd = cmd .. rrd_metric_legend ( metric .. "0" )
            end
        end
    end
    

    --cmd = cmd .. " " .. out_shape .. ":rel_noise" .. phy .. "_norm" .. colors[#metrics+1] .. ":noise" .. phy
    --cmd = cmd .. rrd_metric_legend ( "noise" .. phy .. "_norm" )

    --cmd = cmd .. " " .. out_shape .. ":rel_idle" .. phy .. "_norm" .. colors[#metrics+2] .. ":idle" .. phy
    --cmd = cmd .. rrd_metric_legend ( "idle" .. phy .. "_norm" )

    -- print highlight for each metric
    if ( shape == 'AREA' and highlight == '1' and 1==2 ) then
        out_shape = "LINE2"
        for i, metric in ipairs ( metrics ) do
            if ( metric ~= busy_metric ) then
                if ( stacked == '1' and i > 1 ) then
                    out_shape = "STACK"
                end
                cmd = cmd .. " " .. out_shape .. ":" .. metric .. phy .. colors2[i] .. " \\\n"
            end
        end
        if ( stacked == '1' ) then
            cmd = cmd .. " " .. out_shape .. ":rel_noise" .. phy .. "_norm" .. colors2[#metrics+1] .. " \\\n"
        end
    end

    -- execute rrdtool
    local rrdtool = io.popen( cmd )
    rrdtool:close()

--    -- write command for debug
    nixio.fs.writefile("/tmp/rrdcmd" .. phy .. ".txt", cmd .. "\n" )
end

-- Controlling for the graph page.
--
-- since we use rrdtool1 (1.0.50) that doesn't provide lua bindings
-- we apply a shell command for rrd graph generation
function powquty_render()

    -- create the image
    local vars  = luci.http.formvalue()
    --local spans = luci.util.split( uci.get( "luci_statistics", "collectd_rrdtool", "RRATimespans" ), "%s+", nil, true )
    local spans = luci.util.split( "1hour 1day 1week 1month 1year", "%s+", nil, true )
   	local span  = vars.timespan or uci.get( "luci_statistics", "rrdtool", "default_timespan" ) or spans[1]

    local powquty_paths = uci.get("powquty", "powquty", "powquty_path") or "/tmp/powquty.log"
    local metrics = {"voltage"}
    local busy_metric = {}
    local rrd_dir = uci.get("luci_statistics", "collectd_rrdtool", "DataDir") or "/tmp/rrd"
    local rrdimg_dir = "/tmp/powquty"
    local hostname = luci.sys.hostname()
    local img_width = '800'
    local img_height = '500'
    local stacked = '0'
    local highlight = '1'
    local shape = 'LINE2'

    -- fixme: check if exists before creating
    lfs.mkdir ( rrdimg_dir )

    local phys = ""
    local index = 1 --just added for testing purpose
    --for index, path in ipairs ( regmon_paths ) do
    for index=1,3 do

		if ( index == 2) then
            metrics = {"frequency"}
        elseif (index == 3) then
        	metrics = {"h3","h5","h7","h9","h11"}   
        end
        local rrdimg = "powquty" .. (index-1) .. ".png"
        --local tailcsv_dir = "tail_csv-powquty" .. (index-1)
        local tailcsv_dir = "tail_csv-powquty0"
        
        if ( index ~= 1) then
            phys = phys .. " "
        end
        phys = phys .. index-1

        --if ( vars.img == nil or tonumber ( vars.img ) == index-1 ) then
            generate_rrdimage ( index-1
                              , rrdimg_dir .. "/" .. rrdimg
                              , span
                              , img_width
                              , img_height
                              , rrd_dir .. "/" .. hostname .. "/" .. tailcsv_dir
                              , metrics
                              , shape
                              , stacked
                              , highlight
                              , busy_metric
                              )
        --end
    --end
	end
	
    -- deliver the image
  	if vars.img then
   		local l12 = require "luci.ltn12"
   		local png = io.open(rrdimg_dir .. "/" .. "powquty" .. (vars.img) .. ".png", "r")
    	if png then
	    	luci.http.prepare_content("image/png")
		    l12.pump.all(l12.source.file(png), luci.http.write)
   		end
        local err = dbfiles
        l12.source.error(err)
    	return
   	end

    -- render page
    if (vars.img == nil) then
        luci.template.render( "powquty/graph", {
            timespans        = spans,
            current_timespan = span,
            metrics          = metrics,
            phys             = phys
    	} )
    end

end
