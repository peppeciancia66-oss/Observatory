from osservatorio_meteo.avvisi import GestoreAvvisi
from osservatorio_meteo.valutazione import Esito


class Registra:
    def __init__(self):
        self.avvisi = []
        self.testi = []

    def avvisa(self, titolo, testo, urgente=False):
        self.avvisi.append(titolo)
        self.testi.append(testo)


def esito(sicuro, guasti=()):
    e = Esito(condizioni_buone=sicuro, sicuro_calcolato=sicuro, sicuro_comunicato=sicuro)
    e.guasti = list(guasti)
    return e


def test_guasto_avvisato_una_volta_poi_ripetuto():
    r = Registra()
    g = GestoreAvvisi(r)
    for i in range(10):   # 20 secondi di guasto con contatore che cambia
        g.aggiorna(esito(False, [f"sensore guasto ({i + 4} letture di fila)"]), i * 2, False, 30)
    assert r.avvisi.count("Stazione meteo: GUASTO") == 1
    assert "4 letture" in r.testi[0]
    g.aggiorna(esito(False, ["sensore guasto (999 letture di fila)"]), 30 * 60 + 1, False, 30)
    assert r.avvisi.count("Stazione meteo: GUASTO") == 2
    g.aggiorna(esito(False), 30 * 60 + 3, False, 30)
    assert r.avvisi[-1] == "Stazione meteo: guasto risolto"


def test_cambi_sicuro_avvisati_solo_in_normale():
    r = Registra()
    g = GestoreAvvisi(r)
    g.aggiorna(esito(True), 0, True, 30)
    g.aggiorna(esito(False), 2, True, 30)
    assert r.avvisi == []
    g.aggiorna(esito(True), 4, False, 30)
    g.aggiorna(esito(False), 6, False, 30)
    assert r.avvisi == ["Meteo: condizioni NON SICURE"]
