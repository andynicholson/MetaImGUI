# MetaImGUI Development Container
FROM ubuntu:22.04

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    # Build tools
    build-essential \
    cmake \
    ninja-build \
    ccache \
    git \
    wget \
    curl \
    # C++ tools
    gcc-12 \
    g++-12 \
    clang-15 \
    clang-format-15 \
    clang-tidy-15 \
    clangd-15 \
    lldb-15 \
    gdb \
    # Libraries
    libglfw3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libcurl4-openssl-dev \
    xorg-dev \
    # Coverage tools
    lcov \
    gcovr \
    # Documentation tools
    doxygen \
    graphviz \
    # Additional utilities
    pkg-config \
    zip \
    unzip \
    tar \
    software-properties-common \
    # For GUI support
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    x11-apps \
    # Shell niceties
    zsh \
    vim \
    nano \
    htop \
    tmux \
    && rm -rf /var/lib/apt/lists/*

# Set up alternatives for clang tools
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-15 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-15 100 && \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-15 100 && \
    update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-15 100 && \
    update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-15 100 && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100

# Install vcpkg (useful for additional dependencies)
RUN git clone https://github.com/Microsoft/vcpkg.git /opt/vcpkg && \
    /opt/vcpkg/bootstrap-vcpkg.sh && \
    ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg

# Set environment variables
ENV CC=/usr/bin/gcc-12
ENV CXX=/usr/bin/g++-12
ENV VCPKG_ROOT=/opt/vcpkg

# Create non-root user
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && rm -rf /var/lib/apt/lists/*

# Configure ccache
RUN mkdir -p /home/$USERNAME/.ccache && \
    chown -R $USERNAME:$USERNAME /home/$USERNAME/.ccache
ENV CCACHE_DIR=/home/$USERNAME/.ccache
ENV PATH="/usr/lib/ccache:$PATH"

# Set working directory
WORKDIR /workspace

# Switch to non-root user
USER $USERNAME

# Set up shell
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended
ENV SHELL=/usr/bin/zsh

# Default command
CMD ["/bin/zsh"]

