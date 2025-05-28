# CloverBootloader Hackintosh and Beyond (Fork)

Este é um fork do [CloverBootloader](https://github.com/CloverHackyColor/CloverBootloader) com o objetivo de modernizar, corrigir bugs e facilitar o uso pela comunidade Hackintosh.

## Objetivos do Fork

- Modernizar o código para padrões atuais
- Melhorar o processo de build e releases
- Facilitar contribuições e colaboração
- Corrigir bugs conhecidos e aumentar a estabilidade

## Como compilar (local)

**Pré-requisitos:**
- macOS (recomendado, pode funcionar em Linux com adaptações)
- GCC 5.3 (ou compatível)
- p7zip instalado (`brew install p7zip`)
- Permissão de execução nos scripts (`chmod +x buildme ebuild.sh`)

**Passos:**
```sh
git clone https://github.com/hnanoto/CloverBootloader-Hackintosh-and-Beyond.git
cd CloverBootloader-Hackintosh-and-Beyond
make Clover
```
Os arquivos de release aparecerão na pasta `CloverPackage/sym` e `CloverPackage/CloverV2/EFI/CLOVER/`.

**Limpeza:**
```sh
make clean
```

## Build automático (CI)

A cada push para a branch principal, o GitHub Actions irá:
- Compilar usando macos-latest
- Gerar artefatos ZIP/7Z/PKG
- Disponibilizar para download como artefato do workflow

Você pode acompanhar os builds na aba "Actions" do GitHub.

## Contribuindo

Para contribuir, veja o arquivo [CONTRIBUTING.md](CONTRIBUTING.md).

## Roadmap Inicial

- [ ] Modernizar código C/C++
- [ ] Corrigir warnings e macros obsoletas
- [ ] Adicionar testes automatizados
- [ ] Atualizar dependências externas
- [ ] Melhorar documentação
- [ ] Criar templates de issues e pull requests

## Créditos

Baseado no trabalho da equipe CloverHackyColor e colaborações da comunidade Hackintosh.