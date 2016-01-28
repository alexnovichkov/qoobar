var _version="1.7.0";
function version() {
document.write(_version);
}
function windows_installer() {
	return ("http://sourceforge.net/projects/qoobar/files/qoobar-"+_version+"/Windows/qoobar-"+_version+".exe");
}
function mac_installer() {
	return ("http://sourceforge.net/projects/qoobar/files/qoobar-"+_version+"/Mac_OS_X/qoobar-"+_version+".dmg");
}
function sources() {
	return ("http://sourceforge.net/projects/qoobar/files/qoobar-"+_version+"/src/qoobar-"+_version+".tar.gz");
}
