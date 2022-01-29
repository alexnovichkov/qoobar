var _version="1.7.0";

(function($) {
	$( "span.version" ).html(function() {
            return _version;
        });
	$("span.windows_installer").html(function(){
		var oldText = $("span.windows_installer").html();
		var path="https://sourceforge.net/projects/qoobar/files/qoobar-"+_version+"/Windows/qoobar-"+_version+".exe";
		return "<a href=\""+path+"/download\">"+oldText+"</a>";
	});
	$("span.windows_old_installer").html(function(){
		var oldText = $("span.windows_old_installer").html();
		var path="https://sourceforge.net/projects/qoobar/files/qoobar-"+_version+"/Windows/qoobar-"+_version+".Qt5.exe";
		return "<a href=\""+path+"/download\">"+oldText+"</a>";
	});
	$("span.mac_installer").html(function(){
		var path="https://sourceforge.net/projects/qoobar/files/qoobar-"+_version+"/Mac_OS_X/qoobar-"+_version+".dmg";
		var oldText = $("span.mac_installer").html();
		return "<a href=\""+path+"/download\">"+oldText+"</a>";
	});
	$("span.appimage_installer").html(function(){
		var path="https://sourceforge.net/projects/qoobar/files/qoobar-"+_version+"/AppImage/qoobar-"+_version+".AppImage";
		var name = "qoobar-"+_version+".AppImage";
		return "<a href=\""+path+"/download\">" + name + "</a>";
	});
	$("span.sources_installer").html(function(){
		var path="https://sourceforge.net/projects/qoobar/files/qoobar-"+_version+"/src/qoobar-"+_version+".tar.gz";
		var name = "qoobar-"+_version+".tar.gz";
		return "<a href=\""+path+"/download\">"+name+"</a>";
	});

})(jQuery);

