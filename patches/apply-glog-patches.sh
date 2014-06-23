protobufutil_SOURCE_DIR=$1

patch --forward -p0 --input=$protobufutil_SOURCE_DIR/patches/glog-build-on-mavericks.diff

true