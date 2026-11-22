{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      llvm = pkgs.llvmPackages_22;
    in
    {
      formatter.${system} = pkgs.nixfmt-tree;

      devShells.${system}.default =
        (pkgs.mkShell.override {
          stdenv = llvm.stdenv;
        })
          {
            packages = with pkgs; [
              cmake
              ninja
              git

              llvm.clang-tools

              llvm.libllvm
              lit
            ];
          };
    };
}
