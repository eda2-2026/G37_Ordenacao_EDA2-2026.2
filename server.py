#!/usr/bin/env python3
"""Servidor local da livraria virtual.

Serve a pagina de web/, executa o programa em C para obter o JSON ja
ordenado pelo bucket sort e aceita o cadastro de novos livros no CSV.
So biblioteca padrao: nada de Flask ou dependencias externas.

Uso: python3 server.py   ->   http://localhost:8000
"""

import csv
import json
import os
import subprocess
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path

PORTA = 8000

# Tudo resolvido a partir da pasta do server.py, para o servidor funcionar
# mesmo quando chamado de outro diretorio.
RAIZ = Path(__file__).resolve().parent
WEB = RAIZ / "web"
CSV_LIVROS = RAIZ / "data" / "livros.csv"

# No Windows o make gera livraria.exe; nos outros sistemas, livraria.
EXECUTAVEL = RAIZ / ("livraria.exe" if os.name == "nt" else "livraria")

TIPOS_VALIDOS = ("Romance", "Misterio", "Aventura")

# Limites dos campos da struct Livro: char nome[50] e char descricao[100]
# guardam 49 e 99 BYTES, mais o '\0' do fim.
NOME_MAX = 49
DESC_MAX = 99

TIPOS_CONTEUDO = {
    ".html": "text/html; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".js": "text/javascript; charset=utf-8",
    ".json": "application/json; charset=utf-8",
    ".csv": "text/csv; charset=utf-8",
    ".svg": "image/svg+xml",
}


# ---------------------------------------------------------------------
# Programa em C
# ---------------------------------------------------------------------

def executar_programa_c():
    """Roda o programa em C. Devolve (texto_json, None) ou (None, erro)."""
    if not EXECUTAVEL.exists():
        return None, (
            f"Executavel '{EXECUTAVEL.name}' nao encontrado. "
            "Rode 'make' na pasta do projeto antes de abrir a pagina."
        )
    if not CSV_LIVROS.exists():
        return None, f"Arquivo de dados nao encontrado: data/{CSV_LIVROS.name}"

    try:
        proc = subprocess.run(
            [str(EXECUTAVEL), str(CSV_LIVROS)],
            capture_output=True,
            timeout=15,
        )
    except subprocess.TimeoutExpired:
        return None, "O programa em C demorou demais e foi interrompido."
    except OSError as erro:
        return None, f"Nao foi possivel executar o programa em C: {erro}"

    # O programa manda erro pelo stderr e sai com codigo != 0.
    if proc.returncode != 0:
        mensagem = proc.stderr.decode("utf-8", "replace").strip()
        return None, mensagem or f"O programa em C saiu com codigo {proc.returncode}."

    texto = proc.stdout.decode("utf-8", "replace")
    try:
        json.loads(texto)  # so para conferir que veio JSON valido
    except json.JSONDecodeError as erro:
        return None, f"O programa em C nao devolveu JSON valido: {erro}"

    return texto, None


# ---------------------------------------------------------------------
# CSV
# ---------------------------------------------------------------------

def proximo_id():
    """Maior id que existe no CSV, mais um."""
    maior = 0
    with open(CSV_LIVROS, encoding="utf-8", newline="") as arquivo:
        leitor = csv.reader(arquivo, delimiter=";")
        next(leitor, None)  # pula o cabecalho
        for linha in leitor:
            if linha and linha[0].strip().isdigit():
                maior = max(maior, int(linha[0]))
    return maior + 1


def limpar(texto):
    """Tira o ';' (separador do CSV) e junta quebras de linha em espaco."""
    texto = texto.replace(";", "")
    texto = texto.replace("\r", " ").replace("\n", " ").replace("\t", " ")
    return " ".join(texto.split())


def validar(dados):
    """Valida o JSON recebido. Devolve (livro, None) ou (None, mensagem)."""
    if not isinstance(dados, dict):
        return None, "Envie um objeto JSON com nome, descricao e tipo."

    nome = limpar(str(dados.get("nome", "")))
    descricao = limpar(str(dados.get("descricao", "")))
    tipo = str(dados.get("tipo", "")).strip()

    if not nome:
        return None, "O nome do livro e obrigatorio."
    if len(nome) > NOME_MAX:
        return None, f"O nome pode ter no maximo {NOME_MAX} caracteres."
    if len(nome.encode("utf-8")) > NOME_MAX:
        return None, (
            f"O nome passa de {NOME_MAX} bytes. Em UTF-8 cada letra "
            "acentuada conta como 2, e o campo em C tem 49 bytes."
        )
    if len(descricao) > DESC_MAX:
        return None, f"A descricao pode ter no maximo {DESC_MAX} caracteres."
    if len(descricao.encode("utf-8")) > DESC_MAX:
        return None, (
            f"A descricao passa de {DESC_MAX} bytes (letras acentuadas "
            "contam 2 em UTF-8)."
        )
    if tipo not in TIPOS_VALIDOS:
        return None, "O tipo deve ser Romance, Misterio ou Aventura."

    return {"nome": nome, "descricao": descricao, "tipo": tipo}, None


def termina_em_nova_linha():
    """True se o CSV ja termina com '\\n' (senao a linha nova colaria)."""
    with open(CSV_LIVROS, "rb") as arquivo:
        if arquivo.seek(0, os.SEEK_END) == 0:
            return True
        arquivo.seek(-1, os.SEEK_END)
        return arquivo.read(1) == b"\n"


def acrescentar_no_csv(livro):
    """Grava o livro no fim do CSV e devolve o livro com o id novo."""
    livro_id = proximo_id()
    falta_quebra = not termina_em_nova_linha()
    linha = "{};{};{};{}\n".format(
        livro_id, livro["nome"], livro["descricao"], livro["tipo"]
    )
    with open(CSV_LIVROS, "a", encoding="utf-8", newline="") as arquivo:
        if falta_quebra:
            arquivo.write("\n")
        arquivo.write(linha)
    return {"id": livro_id, **livro}


# ---------------------------------------------------------------------
# Rotas
# ---------------------------------------------------------------------

class Manipulador(BaseHTTPRequestHandler):
    server_version = "LivrariaLocal/1.0"

    # ---- respostas ----

    def responder(self, codigo, corpo, tipo_conteudo):
        if isinstance(corpo, str):
            corpo = corpo.encode("utf-8")
        self.send_response(codigo)
        self.send_header("Content-Type", tipo_conteudo)
        self.send_header("Content-Length", str(len(corpo)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(corpo)

    def responder_json(self, codigo, objeto):
        self.responder(
            codigo,
            json.dumps(objeto, ensure_ascii=False),
            "application/json; charset=utf-8",
        )

    def erro(self, codigo, mensagem):
        self.responder_json(codigo, {"erro": mensagem})

    # ---- GET ----

    def do_GET(self):
        caminho = self.path.split("?", 1)[0]

        if caminho == "/api/ordenar":
            texto, erro = executar_programa_c()
            if erro:
                self.erro(500, erro)
            else:
                self.responder(200, texto, "application/json; charset=utf-8")
        elif caminho in ("/", "/index.html"):
            self.servir_arquivo("index.html")
        else:
            self.servir_arquivo(caminho.lstrip("/"))

    def servir_arquivo(self, nome):
        """Serve um arquivo de web/. Aceita so nomes simples: sem barra e
        sem comecar com ponto, para ninguem escapar da pasta com '..'."""
        if not nome or "/" in nome or "\\" in nome or nome.startswith("."):
            self.erro(404, "Arquivo nao encontrado.")
            return

        caminho = WEB / nome
        if not caminho.is_file():
            self.erro(404, f"Arquivo nao encontrado: {nome}")
            return

        tipo = TIPOS_CONTEUDO.get(caminho.suffix.lower(), "application/octet-stream")
        self.responder(200, caminho.read_bytes(), tipo)

    # ---- POST ----

    def do_POST(self):
        if self.path.split("?", 1)[0] != "/api/livros":
            self.erro(404, "Rota nao encontrada.")
            return

        try:
            tamanho = int(self.headers.get("Content-Length") or 0)
        except ValueError:
            tamanho = 0

        if tamanho <= 0:
            self.erro(400, "Corpo da requisicao vazio.")
            return
        if tamanho > 10_000:
            self.erro(400, "Corpo da requisicao muito grande.")
            return

        try:
            dados = json.loads(self.rfile.read(tamanho).decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError):
            self.erro(400, "JSON invalido.")
            return

        livro, mensagem = validar(dados)
        if mensagem:
            self.erro(400, mensagem)
            return

        try:
            criado = acrescentar_no_csv(livro)
        except OSError as falha:
            self.erro(500, f"Nao foi possivel gravar no CSV: {falha}")
            return

        self.responder_json(201, criado)


# ---------------------------------------------------------------------

def main():
    if not EXECUTAVEL.exists():
        print(f"Aviso: '{EXECUTAVEL.name}' ainda nao existe. Rode 'make' para compilar.",
              flush=True)

    servidor = HTTPServer(("127.0.0.1", PORTA), Manipulador)
    print(f"Livraria virtual em http://localhost:{PORTA}")
    print("Ctrl+C para parar.", flush=True)
    try:
        servidor.serve_forever()
    except KeyboardInterrupt:
        print("\nServidor encerrado.")
    finally:
        servidor.server_close()


if __name__ == "__main__":
    main()
