#!/usr/bin/env bash
set -eux

os_release="$(cat /etc/redhat-release | tr -dc '0-9' | cut -b 1-2)"
case "$os_release" in
  7*) distro=el7 ;;
  8*) distro=el8 ;;
  34*) distro=fc34 ;;
  35*) distro=fc35 ;;
  *) echo "This Linux distribution is not supported" && exit 1 ;;
esac
echo "Building packages for '$distro'"

# Directories used by this script
output_dir=$(pwd)
source_dir=$(dirname "$0")
working_dir=/tmp/saunafs_rpm_working_directory

# Create an empty working directory for rpmbuild
rm -rf "$working_dir"
mkdir -p "$working_dir"/{BUILD,SOURCES,SPECS,RPMS,SRPMS}

# Create a source tarball (the same as those available on Github) using git-archive to make
# sure there are no additional files included in the source package
cd "$source_dir"
version=$(git show HEAD:rpm/saunafs.spec | awk '/^Version:/ {print $2}')
release="saunafs-$version"
git archive --prefix="$release"/ --format=tar HEAD | gzip > "$working_dir/SOURCES/$release.tar.gz"
git show HEAD:rpm/saunafs.spec | sed -e "s/@DISTRO@/$distro/" > "$working_dir/SPECS/saunafs.spec"

# Build packages
cd "$working_dir/SPECS"
rpmbuild --define "_topdir $working_dir" -ba saunafs.spec

# Copy all the created files and clean up
cp "$working_dir"/{RPMS/x86_64,SRPMS}/* "$output_dir"
rm -rf "$working_dir"
