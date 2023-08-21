// Einbinden aller wichtigen Bibliotheken zur Funktionserfüllung
#include <QApplication>
#include <QWidget>
#include <qcustomplot.h>
#include <complex>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>
#include <cmath>
#include <QFileDialog>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QHBoxLayout>

// Erweiterte Eingabemäglichkeit der grundlegenden Übertragungsfunktion mittels Drop-Down-Menü

enum Übertragungsfunktionstyp {
    ErsterOrdnung,
    ZweiterOrdnung,
    DritterOrdnung
};

Übertragungsfunktionstyp ÜbertragungsfunktionstypAuswählen() {
    QStringList items;
    items << "k/(s+1)" << "k/(s+1)^2" << "k/(s+1)^3";

    bool ok;
    QString item = QInputDialog::getItem(nullptr, "Übertragungsfunktionstyp auswählen", "Wähle den Übertragungsfunktionstyp:", items, 0, false, &ok);

    if (ok) {
        if (item == "k/(s+1)") {
            return ErsterOrdnung;
        } else if (item == "k/(s+1)^2") {
            return ZweiterOrdnung;
        } else if (item == "k/(s+1)^3") {
            return DritterOrdnung;
        }
    }

    return ErsterOrdnung;
}

std::complex<double> Übertragungsfunktion(double frequenz, const std::complex<double> &koeffizient, Übertragungsfunktionstyp typ)
{
    std::complex<double> s(0, 2 * M_PI * frequenz);

    switch (typ) {
        case ErsterOrdnung:
            return koeffizient / (s + 1.0);
        case ZweiterOrdnung:
            return koeffizient / ((s + 1.0) * (s + 1.0));
        case DritterOrdnung:
            return koeffizient / ((s + 1.0) * (s + 1.0) * (s + 1.0));
        default:
            return koeffizient / (s + 1.0);
    }
}
// Start der Main Funktion
// Teil 1: Erstellen des Rahmens für das Bode-Diagramm und Schleife mit Dialog für Eingabe der Übertragungsfunktion entsprechend des Konzeptes (Überprüfung der Form)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QWidget fenster;
    fenster.setWindowTitle("Bode-Diagramm");

    QCustomPlot *benutzerdefiniertesDiagramm = new QCustomPlot(&fenster);
    benutzerdefiniertesDiagramm->setObjectName(QStringLiteral("benutzerdefiniertesDiagramm"));

    std::complex<double> koeffizient;
    Übertragungsfunktionstyp funktionstyp;


    funktionstyp = ÜbertragungsfunktionstypAuswählen();

    bool gültigeEingabe = false;

    while (!gültigeEingabe) {

        bool ok;
        QString übertragungsfunktionStr = QInputDialog::getText(&fenster, "Übertragungsfunktion eingeben", "Die Grundfunktion hat die Form K/(s+1), gebe K in der Form a+bj an:", QLineEdit::Normal, QString(), &ok);
            if (!ok || übertragungsfunktionStr.isEmpty()) {
            return 0;
        }

        if (übertragungsfunktionStr.contains("+") && übertragungsfunktionStr.contains("j")) {
            QStringList koeffizientTeile = übertragungsfunktionStr.split('+');
            if (koeffizientTeile.size() == 2) {
                bool ok1, ok2;
                double realTeil = koeffizientTeile[0].toDouble(&ok1);
                double imagTeil = koeffizientTeile[1].left(koeffizientTeile[1].length() - 2).toDouble(&ok2);
                if (ok1 && ok2) {
                    koeffizient = std::complex<double>(realTeil, imagTeil);
                    gültigeEingabe = true;
                } else {
                    QMessageBox::warning(&fenster, "Ungültige Eingabe", "Die Eingabe hatte nicht die korrekte Form. Bitte gib den Koeffizienten in der Form a+bj ein.");
                }
            } else {
                QMessageBox::warning(&fenster, "Ungültige Eingabe", "Die Eingabe hatte nicht die korrekte Form. Bitte gib den Koeffizienten in der Form a+bj ein.");
            }
        } else {
            QMessageBox::warning(&fenster, "Ungültige Eingabe", "Die Eingabe hatte nicht die korrekte Form. Bitte gib den Koeffizienten in der Form a+bj ein.");
        }
    }

    // Teil 2: Mittels Vektoren werden Amplitudengang, Phasengang und Frequenz in der üblichen Form (dB/Grad) berechnet
    QVector<double> frequenzen, amplitudengang, phasengang;

    for (double freq = 0.1; freq <= 1000; freq *= 1.1) {
        frequenzen.append(freq);

        std::complex<double> uf = Übertragungsfunktion(freq, koeffizient, funktionstyp);
        amplitudengang.append(20 * std::log10(std::abs(uf)));
        phasengang.append(std::arg(uf) * 180.0 / M_PI);
    }
    // Teil 3: Die vorher ermittelten Vektoren für Phasengang und Amplitudengang werden in unterschiedlichen Farben im Diagramm hinzugefügt.
    // Weiterhin werden Beschriftungen und Skalierungen der Achsen in der üblichen Form (x-Achse logarithmisch) gesetzt
    benutzerdefiniertesDiagramm->addGraph();
    benutzerdefiniertesDiagramm->graph(0)->setData(frequenzen, amplitudengang);
    benutzerdefiniertesDiagramm->graph(0)->setPen(QPen(Qt::blue));

    benutzerdefiniertesDiagramm->addGraph(benutzerdefiniertesDiagramm->xAxis, benutzerdefiniertesDiagramm->yAxis2);
    benutzerdefiniertesDiagramm->graph(1)->setData(frequenzen, phasengang);
    benutzerdefiniertesDiagramm->graph(1)->setPen(QPen(Qt::red));

    benutzerdefiniertesDiagramm->xAxis->setLabel("Frequenz (log)");
    benutzerdefiniertesDiagramm->yAxis->setLabel("Amplitudengang (dB)");
    benutzerdefiniertesDiagramm->yAxis2->setLabel("Phasengang (Grad)");

    benutzerdefiniertesDiagramm->xAxis->setScaleType(QCPAxis::stLogarithmic);
    benutzerdefiniertesDiagramm->yAxis->setScaleType(QCPAxis::stLinear);
    benutzerdefiniertesDiagramm->yAxis2->setScaleType(QCPAxis::stLinear);

    // Teil 4: Stabilitätsspannen (Amplitudenreserve, Phasenreserve) werden berechnet und im Diagramm als zusätzlicher Text angezeigt.
    QString amplitudenreserveText = "Amplitudenreserve: " + QString::number(1 - std::pow(10, amplitudengang.last() / 20), 'g', 3);
    QString phasenreserveText = "Phasenreserve: " + QString::number(phasengang.last() - (-180.0), 'g', 3);

    benutzerdefiniertesDiagramm->plotLayout()->insertRow(0);
    benutzerdefiniertesDiagramm->plotLayout()->addElement(0, 0, new QCPTextElement(benutzerdefiniertesDiagramm, amplitudenreserveText, QFont("sans", 10, QFont::Bold)));
    benutzerdefiniertesDiagramm->plotLayout()->insertRow(1);
    benutzerdefiniertesDiagramm->plotLayout()->addElement(1, 0, new QCPTextElement(benutzerdefiniertesDiagramm, phasenreserveText, QFont("sans", 10, QFont::Bold)));

    // Anzeigen einer zusätzlichen y-Achse für den Graph des Frequenzganges
    benutzerdefiniertesDiagramm->axisRect()->setupFullAxesBox();
    benutzerdefiniertesDiagramm->yAxis2->setVisible(true);

    // Visualisierungsbereiche der x- und y-Achsen festlegen
    benutzerdefiniertesDiagramm->xAxis->setRange(0.1, 1000);
    benutzerdefiniertesDiagramm->yAxis->setRange(-40, 40);
    benutzerdefiniertesDiagramm->yAxis2->setRange(-200, 200);

    QVBoxLayout *layout = new QVBoxLayout(&fenster);
    layout->addWidget(benutzerdefiniertesDiagramm);
    // Teil 5: Festlegen eines "Diagramm speichern" Buttons
    QPushButton *speichernButton = new QPushButton("Diagramm speichern");
    layout->addWidget(speichernButton);

    fenster.setLayout(layout);
    fenster.show();
    // Wenn "Diagramm speichern" Button gedrückt wird hier die Ausgabe als PNG veranlasst.
    QObject::connect(speichernButton, &QPushButton::clicked, [&](){
        QString dateiPfad = QFileDialog::getSaveFileName(&fenster, "Diagramm speichern", "", "PNG-Dateien (*.png)");
        if (!dateiPfad.isEmpty()) {
            benutzerdefiniertesDiagramm->savePng(dateiPfad);
            QMessageBox::information(&fenster, "Speichern erfolgreich", "Das Diagramm wurde erfolgreich als PNG-Datei gespeichert.");
        }
    });

    return a.exec();
}
