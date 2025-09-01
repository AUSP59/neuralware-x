Name:           neuralware-x
Version:        1.0.0
Release:        1%{?dist}
Summary:        NEURALWARE-X core library and tools
License:        ASL 2.0
URL:            https://example.com
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake ninja-build gcc-c++
%description
Production-grade C++ core and tools.

%prep
%setup -q

%build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

%install
cmake --install build --prefix %{buildroot}/usr

%files
/usr/bin/*
/usr/lib/*
/usr/include/*
