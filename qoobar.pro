TEMPLATE = subdirs

DEFINES += QOOBAR_PORTABLE

SUBDIRS  = src/qoobar_app
SUBDIRS += src/plugins/beatport \
           src/plugins/discogs \
           src/plugins/musicbrainz \
           src/plugins/gd3 \
           src/plugins/playlists


