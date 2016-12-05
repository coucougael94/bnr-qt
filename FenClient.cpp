#include "FenClient.h"

#include <QMessageBox>
#include <QObject>

#define STYLESHEETBUTTON_WHITE "background-color: rgb(240, 240, 240); border-radius: 5px;"
    /// rien
#define STYLESHEETBUTTON_BLUE "background-color: rgb(50, 50, 250); border-radius: 5px;"
    /// petit repère (= rien)
#define STYLESHEETBUTTON_RED "background-color: rgb(250, 50, 50); border-radius: 5px;"
    // touché
#define STYLESHEETBUTTON_WATER "background-color: rgb(50, 250, 250); border-radius: 5px;"
    // a l'eau
#define STYLESHEETBUTTON_BATEAU "background-color: rgb(50, 250, 50); border-radius: 5px;"
    // bateau

FenClient::FenClient():tailleMessage(0),m_aMoiDeJouer('D')
{
    setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(socket, SIGNAL(connected()), this, SLOT(connecte()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(deconnecte()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(erreurSocket(QAbstractSocket::SocketError)));

    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    on_boutonConnexion_clicked();
    int nbTour=0;
    for(int i=1;i <= 9;i++)
    {
        for(int j=1;j<=9;j++)
        {
            boutonAttaque.push_back(new QPushButton());
            gridLayout_1->addWidget(boutonAttaque[nbTour],i,j);
            boutonAttaque[nbTour]->setStyleSheet(STYLESHEETBUTTON_WHITE);
            QObject::connect(boutonAttaque[nbTour],SIGNAL(released()),signalMapper,SLOT(map()));
            signalMapper->setMapping(boutonAttaque[nbTour],nbTour);
            boutonAttaque[nbTour]->setMinimumHeight(40);
            boutonAttaque[nbTour]->setMinimumWidth(20);
            nbTour++;
        }
    }
    QObject::connect(signalMapper,SIGNAL(mapped(int)),this,SLOT(boutonAttaqueClicked(int)));
    nbTour=0;
    QSignalMapper* signalMapper2 = new QSignalMapper (this) ;
    for(int i=1;i <= 9;i++)
    {
        for(int j=1;j<=9;j++)
        {
            boutonDefense.push_back(new QPushButton());
            gridLayout_2->addWidget(boutonDefense[nbTour],i,j);
            boutonDefense[nbTour]->setStyleSheet(STYLESHEETBUTTON_WHITE);
            QObject::connect(boutonDefense[nbTour],SIGNAL(released()),signalMapper2,SLOT(map()));
            signalMapper2->setMapping(boutonDefense[nbTour],nbTour);
            boutonDefense[nbTour]->setMinimumHeight(40);
            boutonDefense[nbTour]->setMinimumWidth(20);
            nbTour++;
        }
    }
    QObject::connect(signalMapper2,SIGNAL(mapped(int)),this,SLOT(boutonDefenseClicked(int)));
    sortirpleinecran->hide();
}
void FenClient::on_boutonConnexion_clicked()
{
    listeMessages->append(tr("<em>Tentative de connexion en cours...</em>"));
    boutonConnexion->setEnabled(false);
    socket->abort(); // On désactive les connexions précédentes s'il y en a
    socket->connectToHost(serveurIP->text(), serveurPort->value()); // On se connecte au serveur demandé
}
void FenClient::on_boutonNettoyer_clicked()
{
    listeMessages->clear();
}
void FenClient::envoie(QString const& messageCache )
{
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    out << (quint16) 0;
    out << messageCache;
    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));

    socket->write(paquet); // On envoie le paquet

    message->clear(); // On vide la zone d'écriture du message
    message->setFocus(); // Et on remet le curseur ā l'intérieur
}
void FenClient::on_boutonEnvoyer_clicked()
{
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    QString messageAEnvoyer=tr("<strong>") + pseudo->text() +tr("</strong> :") + message->text();
    out << (quint16) 0;
    out << messageAEnvoyer;
    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));

    socket->write(paquet); // On envoie le paquet

    message->clear(); // On vide la zone d'écriture du message
    message->setFocus(); // Et on remet le curseur ā l'intérieur
}
void FenClient::on_message_returnPressed()
{
    on_boutonEnvoyer_clicked();
}
void FenClient::donneesRecues()
{
    QDataStream in(socket);

    if (tailleMessage == 0)
    {
        if (socket->bytesAvailable() < (int)sizeof(quint16))
             return;

        in >> tailleMessage;
    }

    if (socket->bytesAvailable() < tailleMessage)
        return;

    QString messageRecu;
    in >> messageRecu;

    decrypte(messageRecu);
    QString retour=messageRecu;

    //on retire tout les $ (Caractère de séparation du script)
    for(int i=0;i < retour.size();i++)
    {
        if(retour[i]=='$')
        {
            i++;
            for(;i<retour.size() || retour[i]!='$';i++)
                retour.remove(i,i+1);
        }
    }
    listeMessages->append(retour);

    // On remet la taille du message ā 0 pour pouvoir recevoir de futurs messages
    tailleMessage = 0;
}
void FenClient::connecte()
{
    listeMessages->append(tr("<em>Connexion réussie !</em>"));
    boutonConnexion->setEnabled(true);
}
void FenClient::deconnecte()
{
    listeMessages->append(tr("<em>Déconnecté du serveur</em>"));
}
void FenClient::erreurSocket(QAbstractSocket::SocketError erreur)
{
    switch(erreur) // On affiche un message différent selon l'erreur qu'on nous indique
    {
        case QAbstractSocket::HostNotFoundError:
            listeMessages->append(tr("<em>ERREUR : le serveur n'a pas pu être trouvé. Vérifiez l'IP et le port.</em>"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            listeMessages->append(tr("<em>ERREUR : le serveur a refusé la connexion. Vérifiez si le programme \"serveur\" a bien été lancé. Vérifiez aussi l'IP et le port.</em>"));
            break;
        case QAbstractSocket::RemoteHostClosedError:
            listeMessages->append(tr("<em>ERREUR : le serveur a coupé la connexion.</em>"));
            break;
        default:
            listeMessages->append(tr("<em>ERREUR : ") + socket->errorString() + tr("</em>"));
    }
    boutonConnexion->setEnabled(true);
}
void FenClient::boutonAttaqueClicked(int indexCase)
{
    if(radioButton_Attaque->isChecked())//attaquer
    {
        envoie("$A"+QString::number(indexCase)+"$");
    }
    else if(radioButton_repere->isChecked())//Repère
    {
        if(boutonAttaque[indexCase]->styleSheet()==STYLESHEETBUTTON_WHITE)//[colone][ligne]
            boutonAttaque[indexCase]->setStyleSheet(STYLESHEETBUTTON_BLUE);
        else if(boutonAttaque[indexCase]->styleSheet()==STYLESHEETBUTTON_BLUE)
            boutonAttaque[indexCase]->setStyleSheet(STYLESHEETBUTTON_WHITE);
    }
}
void FenClient::boutonDefenseClicked(int indexCase)
{
    if(indexCase>81)
        indexCase-=81;
    if(radioButton_bateau->isChecked())
    {//positionnement
        if(boutonDefense[indexCase]->styleSheet()!=STYLESHEETBUTTON_BATEAU)
            boutonDefense[indexCase]->setStyleSheet(STYLESHEETBUTTON_BATEAU);
        else
            boutonDefense[indexCase]->setStyleSheet(STYLESHEETBUTTON_WHITE);
    }
    else if(radioButton_repere1->isChecked())//Repère
    {
        if(boutonDefense[indexCase]->styleSheet()==STYLESHEETBUTTON_WHITE)
            boutonDefense[indexCase]->setStyleSheet(STYLESHEETBUTTON_BLUE);
        else if(boutonDefense[indexCase]->styleSheet()==STYLESHEETBUTTON_BLUE)
            boutonDefense[indexCase]->setStyleSheet(STYLESHEETBUTTON_WHITE);
    }
}

void FenClient::on_jeu_clicked()
{
    ///D :rien (default)
    ///V a moi de jouer
    ///F Ce n'est pas a moi de joueur
    jeu->setEnabled(false);
    radioButton_bateau->setEnabled(false);
    radioButton_Attaque->setEnabled(true);
    radioButton_repere1->setChecked(true);
    qDebug()<<"A";
    if(m_aMoiDeJouer=='F'){//l'autre attend, et je termine de le faire attendre
        qDebug()<<"B";
        label_37->setText(tr("<h1>Vous commençerez 2eme.</h1>"));
        label_34->setText(tr("<h4>L'autre joueur joue 1er</h4>"));
    }
    else if(m_aMoiDeJouer=='D'){//je commence
        qDebug()<<"C";
        m_aMoiDeJouer='V';
        label_37->setText(tr("<h1>Vous commençerez 1er.</h1>"));
        label_34->setText(tr("<h4>L'autre joueur place ces bateaux et joue 2eme</h4>"));
        qDebug()<<"D";
    }
    qDebug()<<"E";
    envoie("$T$");
}
void FenClient::on_pushButton_bug_clicked()
{
    envoie("");
}
void FenClient::decrypte(const QString &messageRecu)
{
    /** SCRIPT:
     * Attaque:
     * $A57$
     * (57= au numéro de la case, 57 n'est qu'un exemple)
     *
     * Defense:
     * $DT57$ //l'adversaire est touché a la case 57.
     * $DR57$ //l'adversaire n'a rien en 57
     * $DE57$ //Une Erreur la case 57 a déja été torpillé.
     */
    for(int index=0;index < messageRecu.size()-1;index++)
    {
        if(messageRecu[index]=='$')
        {
            index++;
            if(messageRecu[index]=='A')
            {//on se fait attaquer !!!
                QString rendu="";
                for(;messageRecu.size() > index && messageRecu[index]!='$';index++)
                {
                    rendu+=messageRecu[index];
                }
                int valeur=rendu.toInt();
                if(valeur<=81 && valeur>=1)//tout va bien
                {
                    if(boutonDefense[valeur]->styleSheet()== STYLESHEETBUTTON_WHITE ||boutonDefense[valeur]->styleSheet()== STYLESHEETBUTTON_BLUE)
                    {
                        envoie("$DR"+QString::number(valeur)+"$");
                        label_37->setText(tr("<h2>L'adversaire a tenter de vous toucher en ")+convertiNUMcaseVersNomCase(valeur)+tr(", mais n'a pas réussi</h2>"));
                        label_34->setText(tr("C'est a vous de jouer"));
                        m_aMoiDeJouer='V';
                    }
                    else if(boutonDefense[valeur]->styleSheet()== STYLESHEETBUTTON_BATEAU)
                    {
                        envoie("$DT"+QString::number(valeur)+"$");
                        label_37->setText(tr("<h2>L'adversaire vous a toucher en ")+convertiNUMcaseVersNomCase(valeur)+tr("</h2>"));
                        label_34->setText(tr("C'est a vous de jouer"));
                        m_aMoiDeJouer='V';
                    }
                    else if(boutonDefense[valeur]->styleSheet()== STYLESHEETBUTTON_WATER || boutonDefense[valeur]->styleSheet()== STYLESHEETBUTTON_RED)
                    {
                        envoie("$DE"+QString::number(valeur)+"$");
                        label_37->setText(tr("<h2>L'adversaire a bugué, il rejoue</h2>"));
                        label_34->setText(tr("A l'adverser de jouer"));
                        m_aMoiDeJouer='F';
                    }
                }
            }
            else if(messageRecu[index]=='D')
            {
                index++;
                if(messageRecu[index]=='T')
                {//l'adversere est touché
                    QString rendu="";
                    for(;messageRecu.size() > index && messageRecu[index]!='$';index++)
                    {
                        rendu+=messageRecu[index];
                    }
                    int valeur=rendu.toInt();
                    if(valeur<=81 && valeur>=1)//tout va bien
                    {
                        label_37->setText(tr("<h2>Vous avez touché l'adverser !</h2>"));
                        label_34->setText(tr("A l'adverser de jouer"));
                        m_aMoiDeJouer='F';
                        boutonAttaque[valeur]->setStyleSheet(STYLESHEETBUTTON_RED);// si l'adversere avez déja attaqué ici, tant pis pour lui !
                    }
                }
                else if(messageRecu[index]=='R')
                {
                    QString rendu="";
                    for(;messageRecu.size() > index && messageRecu[index]!='$';index++)
                    {
                        rendu+=messageRecu[index];
                    }
                    int valeur=rendu.toInt();
                    if(valeur<=81 && valeur>=1)//tout va bien
                    {
                        label_37->setText(tr("<h2>Vous n'avez pas touché l'adverser</h2>"));
                        label_34->setText(tr("A l'adverser de jouer"));
                        m_aMoiDeJouer='F';
                        boutonAttaque[valeur]->setStyleSheet(STYLESHEETBUTTON_WATER);// si l'adversere avez déja attaqué ici, tant pis pour lui !
                    }
                }
            }
            else if(messageRecu[index]=='T')
            {
                label_37->setText(tr("<h2>L'autre joueur a terminé de placer ces bateaux</h2>"));
                qDebug()<<"1";
                if(m_aMoiDeJouer=='V')
                    label_34->setText(tr("Vous pouvez jouer"));
                else
                    label_34->setText(tr("L'autre joueur commencera premier"));
                qDebug()<<"2";
                m_aMoiDeJouer=='F';
                qDebug()<<"3";
            }
        }
    }
}

/*
Note : cet fonction peut se faire remplacer par des simplifications :
-des if < AND >
-des % 9
*/
QString FenClient::convertiNUMcaseVersNomCase(int numDeCase)
{
    if(numDeCase >= 82)
        numDeCase -= 81;
    switch(numDeCase){
        case 0:
            return "Erreur";
            break;
        case 1:
            return "A1";
            break;
        case 2:
            return "B1";
            break;
        case 3:
            return "C1";
            break;
        case 4:
            return "D1";
            break;
        case 5:
            return "E1";
            break;
        case 6:
            return "F1";
            break;
        case 7:
            return "G1";
            break;
        case 8:
            return "H1";
            break;
        case 9:
            return "I1";
            break;
        case 10:
            return "A2";
            break;
        case 11:
            return "B2";
            break;
        case 12:
            return "C2";
            break;
        case 13:
            return "D2";
            break;
        case 14:
            return "E2";
            break;
        case 15:
            return "F2";
            break;
        case 16:
            return "G2";
            break;
        case 17:
            return "H2";
            break;
        case 18:
            return "I2";
            break;
        case 19:
            return "A3";
            break;
        case 20:
            return "B3";
            break;
        case 21:
            return "C3";
            break;
        case 22:
            return "D3";
            break;
        case 23:
            return "E3";
            break;
        case 24:
            return "F3";
            break;
        case 25:
            return "G3";
            break;
        case 26:
            return "H3";
            break;
        case 27:
            return "I3";
            break;
        case 28:
            return "A4";
            break;
        case 29:
            return "B4";
            break;
        case 30:
            return "C4";
            break;
        case 31:
            return "D4";
            break;
        case 32:
            return "E4";
            break;
        case 33:
            return "F4";
            break;
        case 34:
            return "G4";
            break;
        case 35:
            return "H4";
            break;
        case 36:
            return "I4";
            break;
        case 37:
            return "A5";
            break;
        case 38:
            return "B5";
            break;
        case 39:
            return "C5";
            break;
        case 40:
            return "D5";
            break;
        case 41:
            return "E5";
            break;
        case 42:
            return "F5";
            break;
        case 43:
            return "G5";
            break;
        case 44:
            return "H5";
            break;
        case 45:
            return "I5";
            break;
        case 46:
            return "A6";
            break;
        case 47:
            return "B6";
            break;
        case 48:
            return "C6";
            break;
        case 49:
            return "D6";
            break;
        case 50:
            return "E6";
            break;
        case 51:
            return "F6";
            break;
        case 52:
            return "G6";
            break;
        case 53:
            return "H6";
            break;
        case 54:
            return "I6";
            break;
        case 55:
            return "A7";
            break;
        case 56:
            return "B7";
            break;
        case 57:
            return "C7";
            break;
        case 58:
            return "D7";
            break;
        case 59:
            return "E7";
            break;
        case 60:
            return "F7";
            break;
        case 61:
            return "G7";
            break;
        case 62:
            return "H7";
            break;
        case 63:
            return "I7";
            break;
        case 64:
            return "A8";
            break;
        case 65:
            return "B8";
            break;
        case 66:
            return "C8";
            break;
        case 67:
            return "D8";
            break;
        case 68:
            return "E8";
            break;
        case 69:
            return "F8";
            break;
        case 70:
            return "G8";
            break;
        case 71:
            return "H8";
            break;
        case 72:
            return "I8";
            break;
        case 73:
            return "A9";
            break;
        case 74:
            return "B9";
            break;
        case 75:
            return "C9";
            break;
        case 76:
            return "D9";
            break;
        case 77:
            return "E9";
            break;
        case 78:
            return "F9";
            break;
        case 79:
            return "G9";
            break;
        case 80:
            return "H9";
            break;
        case 81:
            return "I9";
            break;
        default:
            QMessageBox::information(this, tr("Erreur"), tr("Contacter le meneur du jeu #defaultNUmversnomcase:")+QString::number(numDeCase));
            return "I9REG";
            break;
    }
}
