package uk.co.msaqib.zumocontrols

import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*
import android.bluetooth.BluetoothAdapter
import android.content.Context
import android.content.Intent
import android.support.v7.app.AlertDialog
import android.view.LayoutInflater
import android.view.View
import android.widget.*


class MainActivity : AppCompatActivity() {

    private lateinit var mBTAdapter: BluetoothAdapter
    private var mName = ""

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val pairedDevicesList = lstPairedDevices

        val mBTArrayAdapter = ArrayAdapter<String>(this, android.R.layout.simple_list_item_1)
        mBTAdapter = BluetoothAdapter.getDefaultAdapter() // get a handle on the bluetooth radio
        val mPairedDevices = mBTAdapter.bondedDevices
        if (mBTAdapter.isEnabled) {
            // put it's one to the adapter
            for (device in mPairedDevices){
                mBTArrayAdapter.add(device.getName() + "\n" + device.getAddress())
            }

            Toast.makeText(applicationContext, "Show Paired Devices", Toast.LENGTH_SHORT).show()
        } else
            Toast.makeText(applicationContext, "Bluetooth not on", Toast.LENGTH_SHORT).show()



        pairedDevicesList.adapter = mBTArrayAdapter // assign model to view
        pairedDevicesList.onItemClickListener = mDeviceClickListener
    }

    val mDeviceClickListener = AdapterView.OnItemClickListener { _, v, _, _ ->
        if (!mBTAdapter.isEnabled) {
            Toast.makeText(baseContext, "Bluetooth not on", Toast.LENGTH_SHORT).show()
            return@OnItemClickListener
        }

        // Get the device MAC address, which is the last 17 chars in the View
        val info = (v as TextView).text.toString()
        val address = info.substring(info.length - 17)

        val intent = Intent(this, ZumoControl::class.java)
        intent.putExtra("bt.device.address", address)
        intent.putExtra("zumo.player.name", mName)
        startActivity(intent)
    }

    override fun onStart() {
        super.onStart()
        if(mName.isNullOrEmpty()){
            showGetNameDialog()
        }else{
            displayPlayerName()
        }
    }

    private fun displayPlayerName(){
        player_name.text = mName
        player_name.visibility = View.VISIBLE
    }

    private fun showGetNameDialog(){
        val view = LayoutInflater.from(this).inflate(R.layout.dialog_name_input, null)
        val editText = view.findViewById<EditText>(R.id.name);
        val btnSubmit = view.findViewById<Button>(R.id.btnSubmitName)

        val dialog = AlertDialog.Builder(this)
            .setTitle("Player Name")
            .setView(view)
            .create()

        dialog.setOnShowListener { dialog
            btnSubmit.setOnClickListener{
                if(editText.text.isNullOrEmpty()){
                    editText.error = "Name is empty"
                }else{
                    mName = editText.text.toString()
                    displayPlayerName()
                    dialog.dismiss()
                }
            }
        }

        dialog.setCancelable(false);
        dialog.show()
    }

    fun convertPixelsToDp(px: Float, context: Context): Int {
        val resources = context.resources
        val metrics = resources.displayMetrics
        return (px / (metrics.densityDpi / 160f)).toInt()
    }
}
