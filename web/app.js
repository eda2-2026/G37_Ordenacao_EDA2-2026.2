/* app.js - liga a pagina ao servidor.
 *
 * GET  /api/ordenar  ->  { chegada: [...], baldes: [ {letra, livros}, ... ] }
 * POST /api/livros   ->  cadastra um livro no CSV
 *
 * JavaScript puro, sem framework. Os elementos sao montados com
 * createElement e textContent, nunca com innerHTML: assim um titulo com
 * '<' ou '&' entra como texto e nao como marcacao. */

"use strict";

var chegada      = document.getElementById("chegada");
var prateleiras  = document.getElementById("prateleiras");
var secaoChegada = document.getElementById("secao-chegada");
var estante      = document.getElementById("estante");
var botaoEstante = document.getElementById("botao-estante");
var botaoSalvar  = document.getElementById("botao-salvar");
var formulario   = document.getElementById("form-livro");
var mensagem     = document.getElementById("mensagem");
var resumo       = document.getElementById("resumo");

/* ------------------------------------------------------------------ */
/* Mensagens                                                          */
/* ------------------------------------------------------------------ */

function avisar(texto, classe) {
  mensagem.textContent = texto;
  mensagem.className = "mensagem" + (classe ? " " + classe : "");
}

/* ------------------------------------------------------------------ */
/* Montagem das lombadas                                              */
/* ------------------------------------------------------------------ */

/* Mesma regra de altura do modelo: titulo curto fica na lombada baixa,
 * titulo longo na alta, para o nome nunca ser cortado.
 * Array.from conta LETRAS, nao bytes - "Drácula" tem 7, nao 8. */
function classeDeAltura(nome) {
  var letras = Array.from(nome).length;
  if (letras <= 14) { return "baixa"; }
  if (letras <= 25) { return "media"; }
  return "alta";
}

/* Uma lombada: cor pelo tipo, altura pelo tamanho do nome, descricao
 * no atributo title (o tooltip nativo do navegador). */
function criarLombada(livro, indice) {
  var span = document.createElement("span");
  span.className = "lombada " + livro.tipo + " " + classeDeAltura(livro.nome);
  /* a cada 3 livros, uma lombada levemente torta */
  if (indice % 3 === 2) {
    span.classList.add("torta");
  }
  span.title = livro.descricao;
  span.textContent = livro.nome;
  return span;
}

/* Uma prateleira: letra do balde, os livros em pe e a barra de madeira.
 * Balde vazio recebe a letra apagada (.vazia) e nenhuma lombada. */
function criarPrateleira(balde, indice) {
  var div = document.createElement("div");
  div.className = "prateleira";
  /* o atraso em cascata e o que da a sensacao de "assentando" um balde
     depois do outro; 27 regras no CSS seria repetitivo demais */
  div.style.animationDelay = (indice * 0.03).toFixed(2) + "s";

  var letra = document.createElement("div");
  letra.className = "letra" + (balde.livros.length === 0 ? " vazia" : "");
  letra.textContent = balde.letra;
  div.appendChild(letra);

  var livros = document.createElement("div");
  livros.className = "livros";
  balde.livros.forEach(function (livro, i) {
    livros.appendChild(criarLombada(livro, i));
  });
  div.appendChild(livros);

  var madeira = document.createElement("div");
  madeira.className = "madeira";
  div.appendChild(madeira);

  return div;
}

/* ------------------------------------------------------------------ */
/* Desenho da pagina                                                  */
/* ------------------------------------------------------------------ */

function desenhar(dados) {
  /* chegada: ordem original do CSV, levemente desalinhada */
  chegada.replaceChildren();
  dados.chegada.forEach(function (livro, i) {
    chegada.appendChild(criarLombada(livro, i));
  });

  /* estante: sempre os 27 baldes, inclusive os vazios */
  prateleiras.replaceChildren();
  dados.baldes.forEach(function (balde, i) {
    prateleiras.appendChild(criarPrateleira(balde, i));
  });

  var ocupados = dados.baldes.filter(function (b) {
    return b.livros.length > 0;
  }).length;

  resumo.textContent =
    dados.chegada.length + " livros recém-chegados, ainda na ordem do arquivo. " +
    ocupados + " dos 27 baldes serão usados.";
}

/* Fecha a estante: volta ao estado inicial, so com a chegada visivel. */
function fecharEstante() {
  estante.classList.remove("aberta");
  botaoEstante.classList.remove("ativo");
  secaoChegada.classList.remove("guardada");
  botaoEstante.textContent = "Visualizar livros";
}

/* ------------------------------------------------------------------ */
/* Dados                                                              */
/* ------------------------------------------------------------------ */

function carregar() {
  botaoEstante.disabled = true;

  return fetch("/api/ordenar")
    .then(function (resposta) {
      return resposta.json().then(function (corpo) {
        if (!resposta.ok) {
          /* o servidor manda { "erro": "..." } */
          throw new Error(corpo.erro || "Falha ao carregar o catálogo.");
        }
        return corpo;
      });
    })
    .then(function (dados) {
      desenhar(dados);
      fecharEstante();
      botaoEstante.disabled = false;
    })
    .catch(function (falha) {
      resumo.textContent = "Não foi possível carregar o catálogo.";
      avisar(falha.message, "erro");
    });
}

/* ------------------------------------------------------------------ */
/* Eventos                                                            */
/* ------------------------------------------------------------------ */

/* Botao: mostra ou esconde a estante. */
botaoEstante.addEventListener("click", function () {
  if (estante.classList.contains("aberta")) {
    fecharEstante();
    return;
  }
  estante.classList.add("aberta");
  botaoEstante.classList.add("ativo");
  secaoChegada.classList.add("guardada");
  botaoEstante.textContent = "Recolher a estante";
});

/* Formulario: manda o livro novo e recarrega tudo. */
formulario.addEventListener("submit", function (evento) {
  evento.preventDefault();  /* sem isso o navegador recarregaria a pagina */

  var livro = {
    nome: document.getElementById("nome").value,
    descricao: document.getElementById("descricao").value,
    tipo: document.getElementById("tipo").value
  };

  botaoSalvar.disabled = true;
  avisar("Gravando...", "");

  fetch("/api/livros", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(livro)
  })
    .then(function (resposta) {
      return resposta.json().then(function (corpo) {
        if (!resposta.ok) {
          /* 400 da validacao: mostra a mensagem que o servidor mandou */
          throw new Error(corpo.erro || "Não foi possível adicionar o livro.");
        }
        return corpo;
      });
    })
    .then(function (criado) {
      formulario.reset();
      avisar('"' + criado.nome + '" entrou no catálogo com o id ' + criado.id + ".", "ok");
      /* recarrega: a estante se fecha e o livro novo aparece na chegada */
      return carregar();
    })
    .catch(function (falha) {
      avisar(falha.message, "erro");
    })
    .then(function () {
      botaoSalvar.disabled = false;
    });
});

/* Ao abrir a pagina: busca os dados e mostra so a chegada. */
carregar();
