package ArduinoSunFollowerController;

import com.fazecast.jSerialComm.SerialPort;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Scanner;
import java.util.concurrent.TimeUnit;

public class GUI extends ArduinoSerialReading {

    static SerialPort arduino;

    public static void main(String argv[]) {

        JFrame window = new JFrame();
        JTextPane pane = new JTextPane();
        JPanel top = new JPanel();
        JButton button = new JButton("Connect");
        JComboBox<String>  ports = new JComboBox<>();
        JTextArea modeIndicator = new JTextArea("Mode: ");

        //full screen
        Dimension DimMax = Toolkit.getDefaultToolkit().getScreenSize();
        window.setMaximumSize(DimMax);
        window.setExtendedState(JFrame.MAXIMIZED_BOTH);

        //setup za velicinu i svojstva prozora
        window.setLayout(new BorderLayout());
        window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        window.setTitle("Graf");
        window.setVisible(true);

        //deklaracija i definicija grafova
        XYSeries vrijednostiLivi = new XYSeries("Lijevo");
        XYSeries vrijednostiDesni = new XYSeries("Desno");
        XYSeries vrijednostiGori = new XYSeries("Gore");
        XYSeries vrijednostiDoli = new XYSeries("Dolje");
        XYSeries avgVrijednostiSenzora = new XYSeries("Avg");
        XYSeries naponNaPanelu = new XYSeries("Napon na ćeliji");

        XYSeries horizontalniMotor = new XYSeries("Horizontalni servo");
        XYSeries vertikalniMotor = new XYSeries("Vertikalni servo");


        //deklaracija i definicija kolekcija za podrucje iscrtavanja grafova
        XYSeriesCollection kolekcijaVrijednostiSenzora = new XYSeriesCollection(vrijednostiDesni);
        kolekcijaVrijednostiSenzora.addSeries(vrijednostiLivi);
        kolekcijaVrijednostiSenzora.addSeries(vrijednostiGori);
        kolekcijaVrijednostiSenzora.addSeries(vrijednostiDoli);
        kolekcijaVrijednostiSenzora.addSeries(avgVrijednostiSenzora);
        //deklaracija grafa i postavke izgleda grafa
        JFreeChart grafOcitavanjaSenzora = ChartFactory.createXYLineChart("Senzor","t [ms]","",kolekcijaVrijednostiSenzora);

        //deklaracija i definicija kolekcije za drugo podrucje iscrtavanja grafova
        XYSeriesCollection naponPanela = new XYSeriesCollection(naponNaPanelu);
        //deklaracija grafa i postavke izgleda grafa
        JFreeChart grafNaponaNaPanelu = ChartFactory.createXYLineChart("Napon", "t [ms]" , " U [V]" , naponPanela);


        //deklaracija i definicija kolekcije za motore iscrtavanja grafova
        XYSeriesCollection motori = new XYSeriesCollection(horizontalniMotor);
        motori.addSeries(vertikalniMotor);
        //deklaracija grafa i postavke izgleda grafa
        JFreeChart grafMotora = ChartFactory.createXYLineChart("Nagib motora", "t [ms]" , " Kut [°]" , motori);


        JPanel center = new JPanel();
        center.add(new ChartPanel(grafNaponaNaPanelu), Box.CENTER_ALIGNMENT);
        center.add(new ChartPanel(grafOcitavanjaSenzora), BorderLayout.NORTH);

        window.add(center,BorderLayout.CENTER);
        center.add(new ChartPanel(grafMotora),BorderLayout.SOUTH);

        top.add(modeIndicator,BoxLayout.X_AXIS);
        top.add(ports,BoxLayout.X_AXIS);
        top.add(button,BoxLayout.X_AXIS);
        window.add(top, BorderLayout.NORTH);

        pane.setAutoscrolls(true);
        window.setVisible(true);

        //thread za citanje podataka na serijski port
        Thread dataRead = new Thread()
        {
            @Override
            public void run() {
                super.run();
                int time = 0;
                int vrijednosti[];

                System.out.println("Read thread");
                if(arduino.isOpen())
                {
                    System.out.println("Arduino connected on port " + arduino.getSystemPortName());
                    arduino.setComPortTimeouts(SerialPort.TIMEOUT_SCANNER,0,0);
                    Scanner inputRead = new Scanner(arduino.getInputStream());

                    //waits for input from serial
                    while(inputRead.hasNextLine())
                    {
                        try {
                            vrijednosti = parse(inputRead.nextLine());

                            //   ispis u konzoli
                            System.out.print("Lijevo " + vrijednosti[0]);
                            System.out.print("\tGore " + vrijednosti[1]);
                            System.out.print("\tDesno " + vrijednosti[2]);
                            System.out.print("\tDolje " + vrijednosti[3]);
                            System.out.print("\tPanel " + vrijednosti[4]);
                            System.out.print("\tHorizontalni Motor " + vrijednosti[5]);
                            System.out.print("\tVertikalni Motor " + vrijednosti[6]);
                            System.out.println();


                            if (vrijednosti[7] == 1)
                            {
                                    modeIndicator.setText("Mode: Automatski");
                            }
                            else
                            {
                                    modeIndicator.setText("Mode: Ručni");
                            }


                            //spremanje vrijednosti i vrimena za x i y os u XYSeries

                            if(vrijednosti[7] == 1)
                            {
                                vrijednostiLivi.add(time++,vrijednosti[0]);
                                vrijednostiGori.add(time++,vrijednosti[1]);
                                vrijednostiDesni.add(time++,vrijednosti[2]);
                                vrijednostiDoli.add(time++,vrijednosti[3]);
                                avgVrijednostiSenzora.add(time++, average(vrijednosti));
                            }

                            naponNaPanelu.add(time++, vrijednosti[4] / 100.0);
                            horizontalniMotor.add(time++,vrijednosti[5]);
                            vertikalniMotor.add(time++,vrijednosti[6]);

                            StringBuilder finalString = new StringBuilder();

                            for (int i = 0; i < vrijednosti.length; i++) {
                                finalString.append(vrijednosti[i] + "\t");
                            }
                            finalString.append("\n");

                            pane.setText(finalString.toString());

                            TimeUnit.MILLISECONDS.sleep(10);
                            //TimeUnit.SECONDS.sleep(1);

                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }
            }
        };


        SerialPort portovi[] = SerialPort.getCommPorts();

        for (int i = 0; i < portovi.length; i++) {
            System.out.println(portovi[i].getSystemPortName());
            ports.addItem(portovi[i].getSystemPortName());
        }

        button.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {


                arduino = SerialPort.getCommPort(ports.getSelectedItem().toString());
                arduino.setComPortParameters(9600,8,1,0);

                if(arduino == null)
                {
                    System.out.println("Arduino nije spojen!");
                    JOptionPane.showMessageDialog(window,"Arduino is not connected!");
                    System.exit(0);
                }

                arduino.openPort();


                if(button.getText().equals("QUIT"))
                {
                    ports.setEnabled(true);
                    dataRead.stop();
                    System.exit(0);
                }

                if(button.getText().equals("Connect"))
                {
                    window.repaint();
                    dataRead.start();
                    button.setText("QUIT");
                    ports.setEnabled(false);
                }

            }
        });



    }

    private static int[] parse(String data)
    {
        String podaci[];
        int vrijednosti[] = new int[8];

        podaci = data.split(";");

        for (String item : podaci)
        {
            item = item.replaceAll(" ","");
        }

        for (int i = 0; i < podaci.length; i++)
        {
            StringBuilder tmpData = new StringBuilder();
            for (int j = 0; j < podaci[i].length(); j++)
            {
                if(Character.isDigit(podaci[i].charAt(j)))
                {
                    tmpData.append(podaci[i].charAt(j));
                }
            }
            vrijednosti[i] = Integer.valueOf(tmpData.toString());
        }

        for (int i = 0; i < vrijednosti.length; i++) {
            if(vrijednosti[i] > 1024)
                vrijednosti[i] = 1023;

            if (vrijednosti[i] < 0)
                vrijednosti[i] = 0;
        }

     return vrijednosti;
    }


    private static float average(int val[])
    {
        float sum = 0;

        for (int i = 0; i < 4; i++) {
            sum = sum + val[i];
        }
        return sum / 4;
    }
}
