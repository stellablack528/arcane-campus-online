# TaleState runtime subset

This directory contains the model, deterministic runtime, JSON loader and
semantic validator from
[TaleState C++](https://github.com/stellablack528/talestate-cpp), commit
`6f8fa9cca5597999869ef9e3090fe7f0a0f9122f`. The upstream MIT license is
included as `LICENSE`. Keep this subset aligned with upstream when updating the
open-world integration.

The provider, agent and save implementation are not vendored here. The game
loads three authored era world packages with TaleState's loader and uses its
rule-checked actions for the first-week incident and open-world routes.
The JSON loader uses the nlohmann JSON 3.11.3 single header under
`include/nlohmann`, licensed under MIT.
