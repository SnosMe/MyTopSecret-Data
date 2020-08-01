#!/bin/bash

gn gen out/Debug &&\
  ninja -C out/Debug &&\
  out/Debug/native-store
